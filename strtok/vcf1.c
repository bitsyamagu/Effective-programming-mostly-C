#include<stdio.h>
#include<string.h>
#include<stdlib.h>

typedef struct {
    char* chrom;
    int pos;
    char* id;
    char* ref;
    char* alt;
    char* qual;
    char* filter;
    char* raw_info;
    char* format;
    char* raw_genotypes;
} VCFEntry;

void dump_vcf_ent(VCFEntry* ent){
    fprintf(stderr, "chrom: %s\n", ent->chrom);
    fprintf(stderr, "pos: %d\n", ent->pos);
    fprintf(stderr, "id: %s\n", ent->id);
    fprintf(stderr, "ref: %s\n", ent->ref);
    fprintf(stderr, "alt: %s\n", ent->alt);
    fprintf(stderr, "qual: %s\n", ent->qual);
    fprintf(stderr, "filter: %s\n", ent->filter);
    fprintf(stderr, "raw_info: %s\n", ent->raw_info);
    fprintf(stderr, "format: %s\n", ent->format);
    fprintf(stderr, "genotypes: %s\n", ent->raw_genotypes);
}
VCFEntry* parse_vcf_line(char* buf){
   char* p = (char*) malloc(strlen(buf));
   strcpy(p, buf);

   VCFEntry* ent = (VCFEntry*)calloc(1, sizeof(VCFEntry));
   char* token = strtok(p, "\t");
   ent->chrom = token;
   for(int i = 1; i<9; i++){
       token = strtok(NULL, "\t");
       // printf("token: %s\n", token);
       switch (i) {
           case 1:
             ent->pos = atoi(token);
             break;
           case 2:
             ent->id = token;
             break;
           case 3:
             ent->ref = token;
             break;
           case 4:
             ent->alt = token;
             break;
           case 5:
             ent->qual = token;
             break;
           case 6:
             ent->filter = token;
             break;
           case 7:
             ent->raw_info = token;
             break;
           case 8:
             ent->format = token;
             ent->raw_genotypes = ent->format + strlen(token) + 1;
             dump_vcf_ent(ent);
             break;
           default:
             fprintf(stderr, "unknown column %s\n", token);
             break;
       }
       if(ent->raw_genotypes){
           break;
       }
   }
   return ent;
}

int main(int argc, char** argv){
    char buf[512];
    FILE* fp = fopen(argv[1], "r");

    while(NULL != fgets(buf, 512, fp)){
        if(buf[0] == '#'){
            // skip comment
            continue;
        }
        VCFEntry* vent = parse_vcf_line(buf);
        // do something on vent
        free(vent);
    }
    fclose(fp);
}
