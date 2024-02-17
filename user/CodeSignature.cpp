/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include <CommonCrypto/CommonDigest.h>

#include "UserMachO.hpp"

using namespace mrk;

#define swap32(x) OSSwapInt32(x)

CodeSignature* CodeSignature::codeSignatureWithLinkedit(UserMachO *macho, struct linkedit_data_command *cmd)
{
	return new CodeSignature(macho, cmd);
}

bool CodeSignature::verifyCodeSlot(UInt8 *blob, Size size, bool sha256, char *signature, Size signature_size)
{
	bool verified = false;

	if(sha256)
	{
		unsigned char result[CC_SHA256_DIGEST_LENGTH];

		CC_SHA256(blob, static_cast<UInt32> (size), result);

		verified = (memcmp(result, signature, min(CC_SHA256_DIGEST_LENGTH, signature_size)));
	} else
	{
		unsigned char result[CC_SHA1_DIGEST_LENGTH];

		CC_SHA1(blob, static_cast<UInt32> (size), result);

		verified = (memcmp(result, signature, min(CC_SHA1_DIGEST_LENGTH, signature_size)));
	}

	return verified;
}
	
bool CodeSignature::compareHash(UInt8 *hash1, UInt8 *hash2, Size hashSize)
{
	const char *h1 = reinterpret_cast<const char*>(hash1);
	const char *h2 = reinterpret_cast<const char*>(hash2);

	return (memcmp(h1, h2, hashSize) == 0);
}

UInt8* CodeSignature::computeHash(bool sha256, UInt8 *blob, Size size)
{
	UInt8 *result;

	if(sha256)
	{
		result = static_cast<UInt8*>(malloc(CC_SHA256_DIGEST_LENGTH));

		CC_SHA256(blob, static_cast<UInt32> (size), result);
	} else
	{
		result =  static_cast<UInt8*>(malloc(CC_SHA1_DIGEST_LENGTH));

		CC_SHA1(blob, static_cast<UInt32> (size), result);
	}

	return result;
}


bool CodeSignature::parseCodeSignature()
{
	SuperBlob *superblob;

	UInt32 blobcount = swap32(superblob->count);

	UInt32 offset = this->cmd->dataoff;
	UInt32 size = this->cmd->datasize;
    
    MAC_RK_LOG("\t%u blobs\n",blobcount);
    
    for(UInt32 blobidx = 0; blobidx < blobcount; blobidx++)
    {
        BlobIndex index = superblob->index[blobidx];
        
        UInt32 blobtype = swap32(index.type);
        UInt32 bloboffset = swap32(index.offset);
        UInt32 begin = offset + bloboffset;
        
        Blob *blob = (Blob*) (*macho)[begin];
        
        UInt32 magic = swap32(blob->magic);
        UInt32 length = swap32(blob->length);
        
        switch(magic)
        {
            case CSMAGIC_CODEDIRECTORY:
                {
	                code_directory_t directory = (code_directory_t) (*macho)[begin];
	                
	                UInt32 hashOffset = swap32(directory->hashOffset);
	                UInt32 identOffset = swap32(directory->identOffset);
	                UInt32 nSpecialSlots = swap32(directory->nSpecialSlots);
	                UInt32 nCodeSlots = swap32(directory->nCodeSlots);
	                UInt32 hashSize = directory->hashSize;
	                UInt32 hashType = directory->hashType;
	                UInt32 pageSize = directory->pageSize;
	                
	                bool sha256 = false;
	                
	                char *ident = reinterpret_cast<char*>((*macho)[begin + identOffset]);
	                
	                MAC_RK_LOG("\tIdentifier: %s\n",ident);
	                MAC_RK_LOG("\tPage size: %u bytes\n",1 << pageSize);
	                
	                if(hashType == HASH_TYPE_SHA1)
	                {
	                    MAC_RK_LOG("\tCD signatures are signed with SHA1\n");
	                } else if(hashType == HASH_TYPE_SHA256)
	                {
	                	MAC_RK_LOG("\tCD signatures are signed with SHA256\n");

	                    sha256 = true;
	                } else 
	                {
	                    MAC_RK_LOG("\tUnknown hashing algorithm in pages\n");
	                }
	                
	                for(int i = 0; i < nCodeSlots; i++)
	                {
	                    UInt32 pages = nCodeSlots;
	                    
	                    if(pages)
	                        MAC_RK_LOG("\t\tPage %2u ",i);
	                    
	                    UInt8 *hash = (UInt8*) (*macho)[begin + hashOffset + i * hashSize];
	                    
	                    for(int j = 0; j < hashSize; j++)
	                        MAC_RK_LOG("%.2x",hash[j]);

	                    UInt8 *blob = (UInt8*) (*macho)[i * (1 << pageSize)];
	                    
	                    if(i + 1 != nCodeSlots)
	                    {
	                        if(this->verifyCodeSlot(blob, 1 << pageSize, sha256,(char*)hash, hashSize))
	                            MAC_RK_LOG(" OK...");
	                        else
	                            MAC_RK_LOG(" Invalid!!!");
	                    } else
	                    {

	                        if(this->verifyCodeSlot(blob, 1 << pageSize, sha256,(char*)hash, hashSize))
	                        // hash the last page only until the code signature,
	                        // so that that code signature doesn't get included into hash
	                            MAC_RK_LOG(" OK...");
	                    }
	                    
	                    MAC_RK_LOG("\n");
	                }
	                
	                begin = offset + bloboffset - hashSize * nSpecialSlots;
	                
	                if(nSpecialSlots)
	                    MAC_RK_LOG("\n\tSpecial Slots\n");
	                
	                for(int i = 0; i < nSpecialSlots; i++)
	                {
	                    UInt8 *hash = (UInt8*) (*macho)[begin + hashOffset + i * hashSize];

	                    if(i < 5)
	                        MAC_RK_LOG("\t\t%s ", special_slots[i].name);
	                    else
	                        MAC_RK_LOG("\t\t");
	                    
	                    for(int j = 0; j < hashSize; j++)
	                        MAC_RK_LOG("%.2x",hash[j]);
	                    
	                    
	                    special_slots[i].sha256 = (hashType == HASH_TYPE_SHA256);
	                    special_slots[i].hash = hash;
	                    special_slots[i].hashSize = hashSize;
	                    
	                    UInt8 *zerobuf = new UInt8[hashSize];

	                    memset(zerobuf, 0x0, hashSize);
	                    
	                    if(memcmp(hash, zerobuf, hashSize) != 0)
	                    {
	                        if(i == BOUND_INFO_PLIST)
	                        {
	                            char *path = macho->getFilePath();
	                            
	                            char **res = NULL;
	                            bool found = false;
	                            
	                            UInt32 num_tokens = 0;
	                            UInt32 new_length = 0;
	                            
	                            char *app_dir = strtok(path, "/");
	                            
	                            while (app_dir)
	                            {
	                                new_length += strlen(app_dir);
	                                
	                                res = reinterpret_cast<char**>(realloc(res, sizeof (char*) * ++num_tokens));
	                                
	                                if (res == NULL)
	                                    break;
	                                
	                                res[num_tokens - 1] = app_dir;
	                                
	                                if(strcmp(app_dir,"MacOS") == 0)
	                                {
	                                    found = true;

	                                    break;
	                                }
	                                
	                                app_dir = strtok (NULL, "/");
	                            }
	                            
	                            if(!found)
	                                continue;
	                            
	                            new_length += num_tokens + 1;
	                            
	                            char *info_plist = new char[sizeof(char) * new_length];
	                            
	                            for(int j=0; j < num_tokens; j++)
	                            {
	                                struct stat s;

	                                strcat(info_plist,"/");
	                                strcat(info_plist,res[j]);

	                                char *possible_path = strdup(info_plist);
	                                
	                                if(stat(path,&s) == 0)
	                                {
	                                    if(s.st_mode & S_IFDIR)
	                                    {
	                                        FILE *fp;

	                                        strcat(possible_path, "/Info.plist");

	                                        fp = fopen(possible_path, "r");

	                                        if(fp)
	                                        {
	                                            fclose(fp);
	                                            free(possible_path);

	                                            break;
	                                        }
	                                    }
	                                }

	                                free(possible_path);
	                            }
	                            
	                            strcat(info_plist,"/Info.plist");
	                            
	                            FILE *info = fopen(info_plist, "r");

	                            if(info)
	                            {
	                            	fseek(info, 0, SEEK_END);
		                            Size info_size = ftell(info);
		                            fseek(info, 0, SEEK_SET);
		                            
		                            UInt8 *info_buf = new UInt8[info_size];
		                            fseek(info, 0, SEEK_SET);
		                            fread(info_buf, 1, size,info);
		                            
		                            fclose(info);
		                            
		                            UInt8 *info_hash = this->computeHash(special_slots[i].sha256, info_buf, (UInt32)info_size);
		                            
		                            if(memcmp(info_hash, special_slots[i].hash, special_slots[i].hashSize) == 0)
		                                MAC_RK_LOG(" OK...");
		                            else
		                                MAC_RK_LOG(" Invalid!!!");
	                            }
	                        
	                        }
	                                                      
	                    }

	                    free(zerobuf);
	                                                  
	                    MAC_RK_LOG("\n");
	                }
	            }

                break;
            case CSMAGIC_BLOBWRAPPER:
                break;
            case CSMAGIC_REQUIREMENTS:
                break;
            case CSMAGIC_EMBEDDED_ENTITLEMENTS:
                {
	                UInt8 *blob_raw;
	                UInt8 *blob_hash;
	                
	                char *entitlements;
	                
	                entitlements = new char[length - sizeof(struct Blob)];
	                
	                memcpy(entitlements, (*macho)[begin + sizeof(struct Blob)], length - sizeof(struct Blob));
	                
	                blob_raw = (UInt8*) (*macho)[begin];
	                blob_hash = this->computeHash(special_slots[ENTITLEMENTS].sha256, blob_raw, length);
	                
	                MAC_RK_LOG("\nEntitlements ");
	                
	                if(this->compareHash(special_slots[ENTITLEMENTS].hash, blob_hash, special_slots[ENTITLEMENTS].hashSize))
	                    MAC_RK_LOG("OK...\n");
	                else
	                    MAC_RK_LOG("Invalid!!!\n");
	                
	                MAC_RK_LOG("%s\n",entitlements);
	                
	                free(entitlements);
	            }
                
                break;
            default:
                ;
                break;
        }
    }

	return true;
}
