#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int GetFromFat(unsigned int pos,unsigned int offset, FILE* fp);
void ls(unsigned int cluster,char* directory,  FILE* fp);
void init(FILE* fp);
void SetDirectory(int cluster, FILE* fp);
int NameToCluster(char* name, FILE* fp);
void read(char* name, int offset,int size, FILE* fp);
void write(char* name, int offset, int size, char* str, FILE* fp);
unsigned int GetOffset(unsigned int cluster);
unsigned int ReturnOffsetCluster(unsigned int cluster);
char* DecimalToHex(int num);
unsigned int HexToDecimal(char a);
void PutToFat(unsigned int pos ,unsigned int offset,char* hex, FILE* fp);
char* RemoveFromString(char* a, int x);
char* SubstringS(char* a, int x);
int create(int cluster,char* name,int attribute, FILE* fp);
void delete(char* name, FILE* fp);
char *getridofspace(char *str);

typedef struct OpenFile{
    char* name;
    int mode;
}OpenFile;

typedef struct File{
    char file_name[12];
    unsigned int file_size;
    unsigned int file_attr;
    unsigned int first_clus_num;
    unsigned int mode;
    int address;
} File;

File Entries[1000];
OpenFile OpenFiles[1000];
int currEntries=0;
int numOpenFiles=0;
int currentCluster;

unsigned int BytesPerSec;
unsigned int SecPerClus ;
unsigned int RsvdSecCnt ;
unsigned int NumFATS ;
unsigned int FATSz32;
unsigned int RootClus;
unsigned int RootDirectorySector;
unsigned int FirstDataSector;

int main(int arg, const char* argv[])
{
    FILE* fp;
    fp = fopen(argv[1], "r+b");
    if(!fp)
    {
        printf("Error");
        fputs("ERROR: File not found.\n", stderr);
        exit(1);
    }

    init(fp);
    char* tokenArr[5];
    for(int i=0; i<5;i++)
    {
        tokenArr[i]="";
    }
    while(1)
    {
        char input[128];
        printf("FAT=> ");
        fgets(input,128,stdin);
        char* token=strtok(input, " ");
        int count=0;
        while(token)
        {
            tokenArr[count]=token;
            token=strtok(NULL, " ");
            count++;
        }

        count--;
        tokenArr[count][strlen( tokenArr[count])-1]=0;

        if(strcmp(tokenArr[0],"exit")==0)
        {
            fclose(fp);
            return 0;
        }

        else if (strcmp(tokenArr[0],"info")==0)
        {
            printf("BS_jmpBoot: 0x%x\n", GetFromFat(0,3,fp));
            printf("BS_OEMName: %i\n", GetFromFat(3,8,fp));
            printf("BytesPerSec: %i\n", BytesPerSec);
            printf("SecPerClus: %i\n", SecPerClus);
            printf("RsvdSecCnt: %i\n", RsvdSecCnt);
            printf("BPB_NumFATs: %i\n", NumFATS);
            printf("BPB_RootEntCnt: %i\n", GetFromFat(17,2,fp));
            printf("BPB_TotSec16: %i\n", GetFromFat(19,2,fp));
            printf("BPB_Media: 0x%x\n", GetFromFat(21,1,fp));
            printf("BPB_FATSz16: %i\n", GetFromFat(22,2,fp));
            printf("BPB_SecPerTrk: %i\n", GetFromFat(24,2,fp));
            printf("BPB_NumHeads: %i\n", GetFromFat(26,2,fp));
            printf("BPB_HiddSec: %i\n", GetFromFat(28,4,fp));
            printf("TotSec32: %i\n", GetFromFat(32,4,fp));
            
            printf("FATSz32: %i\n", FATSz32);
            printf("BPB_ExtFlags: %i\n", GetFromFat(40,2,fp));
            printf("BPB_FSVer: %i\n", GetFromFat(42,2,fp));
            printf("BPB_RootClus: %i\n", GetFromFat(44,4,fp));
            printf("BPB_FSInfo: %i\n", GetFromFat(48,2,fp));
            printf("BPB_BkBootSec: %i\n", GetFromFat(50,2,fp));
            printf("BPB_Reserved: %i\n", GetFromFat(52,12,fp));
            printf("BS_DrvNum: %i\n", GetFromFat(64,1,fp));
            printf("BS_Reserved1: %i\n", GetFromFat(65,1,fp));
            printf("BS_BootSig: 0x%x\n", GetFromFat(66,1,fp));
            printf("BS_VolID: %i\n", GetFromFat(67,4,fp));
            printf("BS_VolLab: %i\n", GetFromFat(71,11,fp));
            printf("BS_FilSysType: %i\n", GetFromFat(82,8,fp));

            printf("RootClus: %i\n", RootClus);
            printf("RootDirectorySector: %i\n", RootDirectorySector);
            printf("FirstDataSector: %i\n", FirstDataSector);
        }

        else if (strcmp(tokenArr[0],"ls")==0)
        {
            if(strcmp(tokenArr[1],"")==0)
                printf("NEED TO ENTER A DIRECTORY\n");

            else
                {
                int clst = NameToCluster(tokenArr[1], fp);
                if (strcmp(tokenArr[1], ".") == 0)
                    clst = currentCluster;

                if (clst == -1) {
                    printf("ERROR DIRNAME NOT FOUND\n");
                    continue;
                }

                ls(clst, "test", fp);
            }
        }

        else if (strcmp(tokenArr[0],"cd")==0)
        {
            printf("cd\n");
            printf("DIRNAME: %s\n", tokenArr[1]);
            if(strcmp(tokenArr[1],"")==0)
                printf("NEED TO ENTER A DIRECTORY\n");

            else {
                int clst = NameToCluster(tokenArr[1], fp);
                if (clst == -1) {
                    printf("ERROR DIRNAME NOT FOUND\n");
                    continue;
                }

                int x = 0;
                for (int i = 0; i < currEntries; i++) {


                    if (strcmp(tokenArr[1], getridofspace(Entries[i].file_name)) == 0) {
                        x = Entries[i].file_attr;
                    }
                }

                if (x != 16) {
                    printf("NOT A DIRECTORY\n");
                    continue;
                }

                SetDirectory(clst, fp);
            }
        }

        else if (strcmp(tokenArr[0],"size")==0)
        {
            int x=0;
            printf("size\n");
            printf("FILENAME: %s\n", tokenArr[1]);
            if(strcmp(tokenArr[1],"")==0)
                printf("ERROR FILENAME NOT FOUND\n");

            for(int i=0;i<currEntries;i++)
            {
                if(strcmp(tokenArr[1],getridofspace( Entries[i].file_name))==0)
                {
                    printf("SIZE IS %i \n", Entries[i].file_size);
                    x++;
                }
            }

            if(x==0)
            {
                printf("ERROR FILENAME NOT FOUND\n");
            }
        }

        else if (strcmp(tokenArr[0],"creat")==0)
        {
            int found=0;

            printf("FILENAME: %s\n", tokenArr[1]);
            if(strcmp(tokenArr[1],"")==0)
                printf("INVALID FILENAME \n");


            for(int i=0;i<currEntries;i++)
            {
                if(strcmp(tokenArr[1],getridofspace( Entries[i].file_name))==0)
                {
                    printf("FILE ALREADY EXISTS \n");
                    found=1;
                }
            }

            if(found==0)
            {
                int xax= create(currentCluster,tokenArr[1],32,fp);
            }
        }

        else if (strcmp(tokenArr[0],"mkdir")==0)
        {
            printf("mkdir\n");

            if(strcmp(tokenArr[1],"")==0)
                printf("INVALID DIRECTORY NAME\n");

            int found=0;
            for(int i=0;i<currEntries;i++)
            {
                if(strcmp(tokenArr[1],getridofspace( Entries[i].file_name))==0)
                {
                    printf("DIRECTORY ALREADY EXISTS \n");
                    found=1;
                }
            }

            if(found==0)
            {
                create(currentCluster,tokenArr[1],16,fp);
            }
        }

        else if (strcmp(tokenArr[0],"open")==0)
        {
            int mode;
            int x=0;
            if(strcmp(tokenArr[2],"r")==0)
                mode=1;

            else if(strcmp(tokenArr[2],"w")==0)
                mode=2;

            else if(strcmp(tokenArr[2],"rw")==0||strcmp(tokenArr[2],"wr")==0)
                mode=3;

            else
            {
                printf("INVALID MODE\n");
                continue;
            }

            int found=0;
            for(int i=0;i<numOpenFiles;i++)
            {
                if(strcmp(tokenArr[1],getridofspace( OpenFiles[i].name))==0)
                {
                    printf("FILE ALREADY OPENED\n");
                    found=1;
                }
            }

            if(found==0) {
                for (int i = 0; i < currEntries; i++) {
                    if (strcmp(tokenArr[1], getridofspace(Entries[i].file_name)) == 0&&Entries[i].file_attr==32) {
                        x++;
                        OpenFiles[numOpenFiles].name = Entries[i].file_name;
                        OpenFiles[numOpenFiles].mode=mode;
                        numOpenFiles++;
                    }
                }

                if (x == 0) {
                    printf("ERROR FILENAME NOT FOUND\n");
                    continue;
                }
            }
        }

        else if (strcmp(tokenArr[0],"close")==0)
        {
           int x=0;

            for(int i=0;i<numOpenFiles;i++)
            {
                if(strcmp(tokenArr[1],OpenFiles[i].name)==0)
                {
                    x++;
                    OpenFiles[i].name=OpenFiles[numOpenFiles-1].name;
                    OpenFiles[i].mode=OpenFiles[numOpenFiles-1].mode;
                    numOpenFiles--;
                }
            }

            if(x==0)
                printf("FILE IS NOT OPEN OR DOES NOT EXIST\n");
        }

        else if (strcmp(tokenArr[0],"read")==0)
        {
            read(tokenArr[1],atoi(tokenArr[2]),atoi(tokenArr[3]), fp);
        }

        else if (strcmp(tokenArr[0],"write")==0)
        {
            write(tokenArr[1],atoi(tokenArr[2]),atoi(tokenArr[3]),tokenArr[4], fp);
        }

        else if (strcmp(tokenArr[0],"rm")==0)
        {

            int found=0;
            if(strcmp(tokenArr[1],"")==0)
                printf("INVALID FILENAME \n");

            else
            {
                for (int i = 0; i < currEntries; i++) 
                {
                    if (strcmp(tokenArr[1], getridofspace(Entries[i].file_name)) == 0) 
                    {
                        if(Entries[i].file_attr==32)
                            delete(tokenArr[1], fp);
                        else
                            found = 1;
                    }
                }

                if (found==1)
                    printf("NOT A FILE \n");
            }
        }

        else if (strcmp(tokenArr[0],"rmdir")==0)//falta
        {
            int found=0;
            printf("rmdir\n");
            printf("DIRNAME: %s\n", tokenArr[1]);
            for(int i=0;i<currEntries;i++)
            {
                if(strcmp(tokenArr[1],getridofspace( Entries[i].file_name))==0)
                {

                    if(Entries[i].file_attr!=16) {
                        printf("NOT A DIRECTORY\n");
                    }

                    else
                    {
                       int prev=currentCluster;
                       SetDirectory(Entries[i].first_clus_num,fp);
                        for(int j=0;j<currEntries;j++)
                        {
                            if((strcmp(".",getridofspace( Entries[j].file_name))!=0)&&(strcmp("..",getridofspace( Entries[j].file_name))!=0)&&(strcmp(".",getridofspace( Entries[j].file_name))!=0))
                            {
                                found++;
                            }
                        }

                        SetDirectory(prev,fp);
                        if(found==0)
                            delete(tokenArr[i],fp);

                        else
                            printf("DIRECTORY NOT EMPTY\n");

                    }
                }
            }
        }

        else
        {
            printf("Command not understood\n");
            int aaa= atoi(tokenArr[1]);
            int b= atoi(tokenArr[2]);
        }
    }
}

char* DecimalToHex(int num)
{
    char hexnum[100];

    int i=0;
    while (num!=0)
    {
        int temp = 0;
        temp = num % 16;
        if (temp < 10)
        {
            hexnum[i] = temp+48;
            i++;
        }

        else
        {
            hexnum[i] = temp+55;
            i++;
        }

        num = num / 16;
    }
    int x=0;
    if(i%2!=0)
    {
        hexnum[i]=48;
        i++;
    }
    char* hexfinal=(char*)malloc(sizeof(char)*i);
    for(int j=i-1; j>=0; j--)
    {
        hexfinal[x]=hexnum[j];
        x++;
    }
    hexfinal[i]='\0';
    return hexfinal;
}

unsigned int HexToDecimal(char a)
{
    if(a=='A')
        return 10;
    else if(a=='B')
        return 11;

    else if(a=='C')
        return 12;
    else if(a=='D')
        return 13;
    else if(a=='E')
        return 14;
    else if(a=='F')
        return 15;
    else
    {
        int x=a - '0';
        return x;
    }
}

void init(FILE* fp)
{
    BytesPerSec = GetFromFat(11,2,fp);
    SecPerClus = GetFromFat(13,1,fp);
    RsvdSecCnt = GetFromFat(14,2,fp);
    NumFATS = GetFromFat(16,1,fp);
    FATSz32 = GetFromFat(36,4,fp);
    RootClus = GetFromFat(44,4,fp);
    RootDirectorySector=0;
    FirstDataSector= RsvdSecCnt+(NumFATS*FATSz32)+RootDirectorySector;
    currentCluster=RootClus;
    unsigned int fsc= ((currentCluster-2)*SecPerClus)+FirstDataSector;
    unsigned int address=fsc*(BytesPerSec*SecPerClus);
    unsigned char dir_name[12];
    for(int i = 0; i < (BytesPerSec*SecPerClus); i+=32)
    {
        File temp;
        if(GetFromFat(address+i+11,1,fp)==15)
            continue;

        else
            temp.file_attr=GetFromFat(address+i+11,1,fp);

        char ch;
        for(int j = 0; j < 11; j++){
            ch = GetFromFat(address+i+j, 1,fp);
            if(ch != 0)
                dir_name[j] = ch;
            else
                dir_name[j] = ' ';
        }

        dir_name[11] = 0;
        if(dir_name[0] == ' ')
            continue;

        strcpy(temp.file_name,dir_name);
        temp.address=address+i;
        temp.file_size=GetFromFat(address+i+28,4,fp);
        unsigned int dir_clus_hi_word = GetFromFat(address+i+20,2,fp);
        unsigned int dir_clus_lo_word = GetFromFat(address+i+26,2,fp);
        temp.first_clus_num = dir_clus_hi_word | dir_clus_lo_word;
        Entries[currEntries]=temp;
        currEntries++;
    }
}

int GetFromFat(unsigned int pos,unsigned int offset, FILE* fp)
{
    unsigned int data = 0, cur;

    pos--;

    while(offset > 0){
        fseek(fp, pos+offset, SEEK_SET);

        offset--;
        cur = fgetc(fp);
        data = data | cur;

        if(offset != 0) {
            data = data << 8;
        }
    }

    return data;
}

void PutToFat(unsigned int pos,unsigned int offset, char* hex, FILE* fp)
{
char reversed[100];
offset=offset*2;

int x=0;
    for(int i=strlen(hex)-2;i>=0;i=i-2)
    {
         reversed[x]=hex[i];
        x++;

         reversed[x]=hex[i+1];
        x++;
    }
    while(x<offset)
    {
        reversed[x]='0';
        x++;
    }
    reversed[x]='\0';

    for(int i=0;i<offset;i=i+2) 
    {
        int total=0;
        total=total+(HexToDecimal(reversed[i])*16)+HexToDecimal(reversed[i+1]);
        fseek(fp, pos, SEEK_SET);
        fputc(total,fp);
        pos++;
    }
}

void EmptyFile(int cluster, FILE* fp)
{
    int clustertodelete=cluster;
    int found=0;
   while (found==0)
   {
       unsigned int x = GetOffset(clustertodelete);
       unsigned int y = ReturnOffsetCluster(clustertodelete);
       unsigned int a = x * (BytesPerSec * SecPerClus);
       int next = GetFromFat(a + y, 8, fp);
       PutToFat(a + y, 8, DecimalToHex(0), fp);
       clustertodelete=next;
       if (clustertodelete >= 268435448)
       {
           found=1;
       }
   }
}

void delete(char* name, FILE* fp)
{
    int created=0;
    int cluster=currentCluster;
    if(cluster==0)
        cluster=2;

    unsigned int fsc= ((cluster-2)*SecPerClus)+FirstDataSector;
    unsigned int address=fsc*(BytesPerSec*SecPerClus);
    unsigned char dir_name[12];
    for (int i = 0; i < (BytesPerSec * SecPerClus); i += 32)
    {
        File temp;

        if (GetFromFat(address + i + 11, 1, fp) == 15)
        {
            continue;
        }

        char ch;
        for (int j = 0; j < 11; j++)
        {
            ch = GetFromFat(address + i + j, 1, fp);

            if (ch != 0)
            {
                dir_name[j] = ch;

            }
            else
                dir_name[j] = ' ';
        }

        dir_name[11] = 0;
        if (strcmp(name,getridofspace(dir_name))==0)
        {
            for (int j = 0; j < 11; j++)
            {
                PutToFat(address + i + j, 1,DecimalToHex(0), fp);
            }

            PutToFat(address + i + 11, 1,DecimalToHex(0), fp);
            PutToFat(address + i + 28, 4,DecimalToHex(0), fp);
             unsigned int dir_clus_hi_word = GetFromFat(address + i + 20, 2, fp);
             unsigned int dir_clus_lo_word = GetFromFat(address + i + 26, 2, fp);
             int clusterused = dir_clus_hi_word | dir_clus_lo_word;
            PutToFat(address + i + 20, 2,DecimalToHex(0), fp);
            PutToFat(address + i + 26, 2,DecimalToHex(0), fp);

            int clustarr[10000];
            int count=0;
             int clustdone=0;
             int next;
             while(clustdone==0)
             {
                 unsigned int x = GetOffset(clusterused);
                 unsigned int y = ReturnOffsetCluster(clusterused);
                 unsigned a = x * (BytesPerSec * SecPerClus);
                 next=GetFromFat(a + y, 8, fp);

                 if(next<268435448)
                 {
                     clusterused =next;
                     clustarr[count]=next;
                     count++;
                 }
                 else
                 {
                     clustarr[count]=next;
                     clustdone=1;
                 }
             }

             for(int i=count-1;i>=0;i--)
             {
                 unsigned int x = GetOffset(clustarr[i]);
                 unsigned int y = ReturnOffsetCluster(clustarr[i]);
                 unsigned a = x * (BytesPerSec * SecPerClus);
                 PutToFat(a+y,8,DecimalToHex(0),fp);
             }

            break;
        }
    }
}

int create(int cluster,char* name,int attribute, FILE* fp)
{
    int created=0;
    if(cluster==0)
        cluster=2;

    unsigned int fsc= ((cluster-2)*SecPerClus)+FirstDataSector;
    unsigned int address=fsc*(BytesPerSec*SecPerClus);
    unsigned char dir_name[12];
    for (int i = 0; i < (BytesPerSec * SecPerClus); i += 32)
    {
        File temp;

        if (GetFromFat(address + i + 11, 1, fp) == 15)
        {
            continue;
        }

        char ch;
        for (int j = 0; j < 11; j++)
        {
            ch = GetFromFat(address + i + j, 1, fp);

            if (ch != 0)
            {
                dir_name[j] = ch;

            }
            else
                dir_name[j] = ' ';
        }

        dir_name[11] = 0;
        if (dir_name[0] == ' ')
        {
            for (int j = 0; j < 11; j++)
            {
                int x=(int)name[j];
                PutToFat(address + i + j, 1,DecimalToHex(x), fp);
            }

            PutToFat(address + i + 11, 1,DecimalToHex(attribute), fp);
            PutToFat(address + i + 28, 4,DecimalToHex(0), fp);
            break;
        }
    }

    unsigned int x = GetOffset(cluster);
    unsigned int y = ReturnOffsetCluster(cluster);
    unsigned a = x * (BytesPerSec * SecPerClus);

    int next;
    int clustfound=0;
    while(clustfound==0)
    {
        next = GetFromFat(a + y, 8, fp);
        if (next == 0) 
        {
            PutToFat(a + y, 8, DecimalToHex(268435448), fp);
            clustfound=1;
        }
        else
        {
            y = y + 4;
        }
    }

    return created;
}

void SetDirectory(int cluster, FILE* fp)
{
    int done=0;
    currEntries=0;


    if(cluster==0)
        cluster=2;

    currentCluster=cluster;
    while(done==0)
    {
    unsigned int fsc= ((cluster-2)*SecPerClus)+FirstDataSector;
    unsigned int address=fsc*(BytesPerSec*SecPerClus);
    unsigned char dir_name[12];

        for (int i = 0; i < (BytesPerSec * SecPerClus); i += 32) {
            File temp;

            if (GetFromFat(address + i + 11, 1, fp) == 15) {

                continue;
            }
            else {

                temp.file_attr = GetFromFat(address + i + 11, 1, fp);
            }
            char ch;
            for (int j = 0; j < 11; j++) {

                ch = GetFromFat(address + i + j, 1, fp);
                if (ch != 0)
                    dir_name[j] = ch;
                else
                    dir_name[j] = ' ';
            }

            dir_name[11] = 0;

            if(dir_name[0] == ' ')
                continue;

                strcpy(temp.file_name, dir_name);

            temp.address=address+i;

            temp.file_size = GetFromFat(address + i + 28, 4, fp);

            unsigned int dir_clus_hi_word = GetFromFat(address + i + 20, 2, fp);
            unsigned int dir_clus_lo_word = GetFromFat(address + i + 26, 2, fp);
            temp.first_clus_num = dir_clus_hi_word | dir_clus_lo_word;
            Entries[currEntries] = temp;
            currEntries++;
        }

        unsigned int x = GetOffset(cluster);
        unsigned int y = ReturnOffsetCluster(cluster);
        unsigned a = x * (BytesPerSec * SecPerClus);
        int possiblecluster = GetFromFat(a + y, 8, fp);

        if(possiblecluster<268435448)
        {
            cluster=possiblecluster;
        }
        else
            done=1;
    }
}

void ls(unsigned int cluster, char* directory, FILE* fp)
{

int prevcluster= currentCluster;
SetDirectory(cluster,fp);

for(int i=0;i<currEntries;i++)
{
    printf("%s\n", getridofspace( Entries[i].file_name));
}

SetDirectory(prevcluster,fp);

}

int NameToCluster(char* name, FILE* fp)
{
    int x=-1;
    for(int i=0;i<currEntries;i++)
    {
        if(strcmp(name,getridofspace( Entries[i].file_name))==0)
        {

            x = Entries[i].first_clus_num;
        }
    }

    return x;
}

void read(char* name, int offset,int size, FILE* fp)
{
    int openfound=0;
    int filefound=0;
    int sizefound=1;
    File temp;
    char msg[size+1];
    memset(msg,0,(size+1)* sizeof(char));
    char xxx[(BytesPerSec * SecPerClus)+10];
    memset(xxx,0,((BytesPerSec * SecPerClus)+10)* sizeof(char));
    for(int i=0;i<currEntries;i++)
    {
        if(strcmp(name,getridofspace( Entries[i].file_name))==0)
        {
            temp = Entries[i];
            filefound++;
            if(Entries[i].file_attr==16)
            {
                printf("NOT A FILE\n");
                filefound=0;
            }
            if(offset+size>Entries[i].file_size)
            {
                printf("INVALID SIZE OF READ\n");
                sizefound--;
            }
        }
    }

    for(int i=0;i<numOpenFiles;i++)
    {
        if(strcmp(name,getridofspace( OpenFiles[i].name))==0)
        {

            openfound++;
            if(OpenFiles[i].mode==2)
            {
                printf("Mode not compatible for read\n");
                openfound=0;
            }
        }
    }

    if(openfound==0||filefound==0||sizefound==0)
    {
        printf("Cant read file\n");
    }

    else
    {
        int done=0;
        int remaining=offset+size;
        int firstread=0;
        unsigned int clusterUsed = temp.first_clus_num;
        while(done==0) 
        {
            unsigned int fcs = ((clusterUsed - 2) * SecPerClus) + FirstDataSector;
            int file_begin = fcs * (BytesPerSec * SecPerClus);
            fseek(fp, file_begin + offset, SEEK_SET);

            if(remaining>(BytesPerSec * SecPerClus))
            {
                if(firstread==0) {
                	fread(msg, 1, (BytesPerSec * SecPerClus), fp); 
                    remaining = remaining - (512 - offset);
                    firstread=1;
                }

                else
                {
                	fread(xxx, 1, BytesPerSec * SecPerClus, fp);
                	strcat(msg,xxx);
                    remaining=remaining-512;
                }

                unsigned int x = GetOffset(clusterUsed);
                unsigned int y = ReturnOffsetCluster(clusterUsed);
                unsigned a = x * (BytesPerSec * SecPerClus);
                clusterUsed = GetFromFat(a + y, 8, fp);
            }

            else
            {
                fread(xxx, 1, remaining, fp);
                strcat(msg,xxx);

                done=1;
            }
        }
    }

    printf("%s\n", msg);
}

void write(char* name, int offset, int size, char* str, FILE* fp)
{
char* finalstr=malloc((1+size)* sizeof(char));
for(int i=0;i<strlen(str);i++)
{
    finalstr[i]=str[i];
}

    for(int i=strlen(finalstr);i<size;i++)
    {
        finalstr[i]='\0';
    }

    finalstr[size]='\0';

    int openfound=0;
    int filefound=0;
    int sizefound=1;
    File temp;
    char msg[size+1];
    char sub[size+1];

    for(int i=0;i<numOpenFiles;i++)
    {
        if (strcmp(name, getridofspace(OpenFiles[i].name)) == 0) {

            openfound++;
            if (OpenFiles[i].mode == 1) {
                printf("Mode not compatible for write\n");
                openfound = 0;
            }
        }
    }

    for(int i=0;i<currEntries;i++)
    {
        if(strcmp(name,getridofspace( Entries[i].file_name))==0&&openfound!=0)
        {
            temp = Entries[i];
            filefound++;

            if(Entries[i].file_attr==16)
            {
                printf("NOT A FILE\n");
                filefound=0;
            }
            if(offset+size>temp.file_size)
            {
                unsigned int fsc= ((currentCluster-2)*SecPerClus)+FirstDataSector;
                unsigned int address=fsc*(BytesPerSec*SecPerClus);
                unsigned int xxz = GetFromFat( Entries[i].address + 28, 4, fp);

                fseek(fp, Entries[i].address+28, SEEK_SET);
                fputc(20,fp);
                GetFromFat(address + i*32 + 28, 4, fp);

                PutToFat(Entries[i].address + 28,4,DecimalToHex(offset+size),fp);
                temp.file_size=offset+size;
                Entries[i].file_size=offset+size;
            }
        }
    }

    if(openfound==0||filefound==0||sizefound==0)
    {
        printf("Cant read file\n");
    }

    else
    {
        char* test1="TEST";
        int done=0;
        int remaining=offset+size;
        int firstread=0;
        int point=0;
        unsigned int clusterUsed = temp.first_clus_num;
        while(done==0)
        {
            unsigned int fcs = ((clusterUsed - 2) * SecPerClus) + FirstDataSector;
            int file_begin = fcs * (BytesPerSec * SecPerClus);
            fseek(fp, file_begin + offset, SEEK_SET);
            if(remaining>512)
            {
                if (firstread == 0)
                {
                    if(offset<512)
                    {
                        char temp1[1+512-offset];
                        memcpy(temp1, &finalstr[0],512-offset);
                        temp1[512-offset]='\0';
                        strcpy(finalstr,RemoveFromString(finalstr,512-offset));

                        fwrite(temp1, 1, 512 - offset, fp);

                    }
                    firstread = 1;
                    remaining = remaining - (512 - offset);
                }

                else 
                {
                        char temp1[1+512];
                        memcpy(temp1, &finalstr[0],512);
                        temp1[512]='\0';
                        strcpy(finalstr,RemoveFromString(finalstr,512));
                        fwrite(temp1, 1, 512, fp);

                    remaining = remaining - 512;

                }

                unsigned int x = GetOffset(clusterUsed);
                unsigned int y = ReturnOffsetCluster(clusterUsed);
                unsigned a = x * (BytesPerSec * SecPerClus);
                int next;
                next=GetFromFat(a + y, 8, fp);
                if(next<268435448&& next>0)
                {
                    clusterUsed =next;
                }
                else
                {
                    int newclus=0;
                    unsigned int init_a=a;
                    unsigned int init_y=y;
                    int count=0;
                    while(newclus==0)
                    {
                        y = y+4;
                        count++;
                        int poss_cluster=GetFromFat(a + y, 8, fp);

                        if (poss_cluster == 0)
                        {
                            newclus=1;
                            PutToFat(init_a + init_y, 8, DecimalToHex(clusterUsed+count),fp);
                            PutToFat(a+y,8,DecimalToHex(268435448),fp);
                            clusterUsed=poss_cluster;
                        }
                    }
                }
            }
            else
            {
                unsigned int fcs = ((clusterUsed - 2) * SecPerClus) + FirstDataSector;
                int file_begin = fcs * (BytesPerSec * SecPerClus);
                fseek(fp, file_begin + offset, SEEK_SET);
                fwrite(finalstr, 1, remaining, fp);
                done=1;
            }
        }
    }

    free(finalstr);
}

unsigned int GetOffset(unsigned int cluster){
    unsigned int Offset;
    unsigned int x;
    Offset = cluster * 4;
    x = RsvdSecCnt + (Offset / BytesPerSec);
    
    return x;
}

char* SubstringS(char* a, int x)
{
    char* sub=malloc(x);
    strncpy(sub,a,x);

    return sub;
}

unsigned int ReturnOffsetCluster(unsigned int cluster){
    unsigned int Offset;
    Offset = cluster * 4;
    return Offset % BytesPerSec;
}

char *getridofspace(char *s)
{
    char *ar;
    while(isspace((unsigned char)*s)) s++;
    if(*s == 0)
        return s;

    ar = s + strlen(s) - 1;
    while(ar > s && isspace((unsigned char)*ar)) ar--;
    ar[1] = '\0';

    return s;
}

char* RemoveFromString(char* a, int x)
{
    char *xs=a;
    for(int i=0;i<x;i++)
        xs++;

    return xs;
}

