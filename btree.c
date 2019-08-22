#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define DEG 54

struct word_meaning
{
	char word[20];
	char meaning[44];
};

struct BTnode
{
	struct word_meaning data[2*DEG-1];
	fpos_t children[2*DEG];
	int currNodes;
	int isLeaf;
};

void splitChild(struct BTnode *x, int i, struct BTnode *y, FILE *fp, fpos_t *end_pos)
{
    	struct BTnode z;
			z.isLeaf = y->isLeaf;
    	z.currNodes = DEG - 1;
    	for (int j = 0; j < DEG - 1; j++)
        	z.data[j] = y->data[j+DEG];

    	if (y->isLeaf == 0)
    	{
        	for (int j = 0; j < DEG; j++)
            		z.children[j] = y->children[j + DEG];
    	}

    	y->currNodes = DEG - 1;

    	for (int j = x->currNodes; j >= i+1; j--)
        	x->children[j+1] = x->children[j];

	x->children[i+1] = *end_pos;
	fsetpos(fp,end_pos);
        fwrite(&z, sizeof(struct BTnode), 1, fp);
	fgetpos(fp,end_pos);

    	for (int j = x->currNodes-1; j >= i; j--)
        	x->data[j+1] = x->data[j];

    	x->data[i] = y->data[DEG - 1];

    	x->currNodes = x->currNodes + 1;
}

void insertNonFull(struct BTnode *root, struct word_meaning k, FILE* fp, fpos_t *end_pos)
{
    	int i = root->currNodes-1;
    	if (root->isLeaf == 1)
    	{
        	while (i >= 0 && strcmp(root->data[i].word, k.word) > 0)
        	{
            		root->data[i+1] = root->data[i];
            		i--;
        	}

        	root->data[i+1] = k;
        	root->currNodes = root->currNodes + 1;
    	}
    	else
    	{
        	while (i >= 0 && strcmp(root->data[i].word, k.word) > 0)
            		i--;
		struct BTnode child;
		fsetpos(fp, &(root->children[i]));
                fread(&child, sizeof(struct BTnode), 1, fp);
        	if (child.currNodes == 2*(DEG)-1)
        	{
            		splitChild(root, i+1, &child, fp, end_pos);
			fsetpos(fp, &(root->children[i]));
                	fwrite(&child, sizeof(struct BTnode), 1, fp);
            		if (strcmp(root->data[i+1].word, k.word) < 0) {
                		i++;
			}
        	}
		fsetpos(fp, &(root->children[i+1]));
                fread(&child, sizeof(struct BTnode), 1, fp);
        	insertNonFull(&child, k, fp, end_pos);
		fsetpos(fp, &(root->children[i+1]));
                fwrite(&child, sizeof(struct BTnode), 1, fp);
    	}
}

void insert(FILE *fp, fpos_t *end_pos, fpos_t *root_pos, struct word_meaning k)
{

        struct BTnode bt;
        fsetpos(fp, root_pos);
	fread(&bt, sizeof(struct BTnode), 1, fp);
	if (bt.currNodes == 2*DEG-1) {
        	struct BTnode s;
                s.isLeaf=0;
           	s.children[0] = *root_pos;
						*root_pos = *end_pos;
            fsetpos(fp,end_pos);
            fwrite(&s, sizeof(struct BTnode), 1, fp);
            fgetpos(fp,end_pos);
            splitChild(&s,0,&bt,fp,end_pos);
            int i = 0;
            if (strcmp(s.data[0].word, k.word) < 0) {
                	i++;
		}
		struct BTnode child;
		fsetpos(fp,&(s.children[i]));
	        fread(&child, sizeof(struct BTnode), 1, fp);
            	insertNonFull(&child, k, fp, end_pos);
		fsetpos(fp, &(s.children[i]));
                fwrite(&child, sizeof(struct BTnode), 1, fp);
		fsetpos(fp, root_pos);
		fwrite(&s, sizeof(struct BTnode), 1, fp);
		fsetpos(fp,&(s.children[0]));
                fwrite(&bt, sizeof(struct BTnode), 1, fp);
        } else {
		insertNonFull(&bt, k, fp, end_pos);
		fsetpos(fp, root_pos);
                fwrite(&bt, sizeof(struct BTnode), 1, fp);
	}

}

void traverse(FILE *fp,fpos_t root)
{
        fsetpos(fp,&root);
        struct BTnode bt;
        fread(&bt, sizeof(struct BTnode), 1, fp);
        int i;
        for (i = 0; i < bt.currNodes; i++)
        {
                if (bt.isLeaf == 0)
                        traverse(fp,bt.children[i]);
                printf("%s : %s ",bt.data[i].word, bt.data[i].meaning);
        }

        if (bt.isLeaf == 0)
                traverse(fp,bt.children[i]);
}

void search(FILE *fp,fpos_t root,char k[],int *flag)
{
        fsetpos(fp,&root);
        struct BTnode bt;
        fread(&bt, sizeof(struct BTnode), 1, fp);
        int i = 0;
        while (i < bt.currNodes && strcmp(k, bt.data[i].word) > 0)
                i++;

        if (strcmp(bt.data[i].word, k) == 0)
	{
		*flag=1;
                printf("%s\n", bt.data[i].meaning);
		return;
	}

        if (bt.isLeaf == 1) {
                printf("not present");
		return;
	}
        search(fp,bt.children[i],k,flag);
}

int main()
{

	struct BTnode bt;
	bt.currNodes = 0;
	bt.isLeaf = 1;

	FILE *fp;

	fpos_t end_pos;
	fpos_t root_pos;
	fp = fopen("data.bin", "rb");
	if(!fp) {
		fp = fopen("data.bin", "wb");
		fgetpos(fp, &root_pos);
		fwrite(&bt, sizeof(struct BTnode), 1, fp);
		fgetpos(fp, &end_pos);
	}
	fclose(fp);
	fp = fopen("data.bin", "rb+");
	fgetpos(fp, &root_pos);
	//fwrite(&bt, sizeof(struct BTnode), 1, fp);
	fgetpos(fp, &end_pos);

	while(1) {
		printf("choose 1\n 1-insert\n 2-search\n 3-exit\n");
		int x;
		scanf("%d", &x);
		if (x == 3) {
			break;
		}
		if (x == 1) {
			struct word_meaning w;
			printf("enter word\n");
			scanf("%s", w.word);
			printf("enter meaning\n");
			scanf("%s", w.meaning);
			insert(fp, &end_pos, &root_pos, w);
		}
		if (x == 2) {
			int flag = 0;
			char s[20];
			printf("enter word to search\n");
			scanf("%s", s);
			search (fp, root_pos, s, &flag);
		}
	}

	fclose(fp);

	return 0;
}
