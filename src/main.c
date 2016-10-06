#include <buffer.h>
#include <stdio.h>
#include <global.h>
#include <util.h>
#include <dictionary.h>
#include <errno.h>
#include <string.h>	
#include <pthread.h>

#define RADIX 256 /* Radix of input data. */
#define WIDTH  12 /* Width of code word.  */


static void lzw_writebytes(buffer_t inbuf, FILE *outfile)
{
	int ch;
	
	/* Read data from file to the buffer. */
	while ((ch = buffer_get(inbuf)) != EOF)
		fputc(ch, outfile);
}

static code_t lzw_init(dictionary_t dict, int radix)
{
	for (int i = 0; i < radix; i++)
		dictionary_add(dict, 0, i, i);
	
	return (radix);
}

static void lzw_compress(buffer_t in, buffer_t out)
{	
	unsigned ch;       /* Working character. */
	int i, ni;         /* Working entries.   */
	code_t code;       /* Current code.      */
	dictionary_t dict; /* Dictionary.        */
	
	dict = dictionary_create(1 << WIDTH);
	
	i = 0;
	code = lzw_init(dict, RADIX);

	/* Compress data. */
	ch = buffer_get(in);
	while (ch != EOF)
	{	
		ni = dictionary_find(dict, i, (char)ch);
		
		/* Find longest prefix. */
		if (ni >= 0)
		{			
			ch = buffer_get(in);
			i = ni;
		
			/* Next character. */
			if (ch != EOF)
				continue;
		}
		
		buffer_put(out, dict->entries[i].code);
		
		if (code == ((1 << WIDTH) - 1))
		{	
			i = 0;
			dictionary_reset(dict);
			code = lzw_init(dict, RADIX);
			buffer_put(out, RADIX);
			continue;
		}
		
		dictionary_add(dict, i, ch, ++code);
		i = 0;
	}
	
	buffer_put(out, EOF);

	dictionary_destroy(dict);
}

static void lzw_readbytes(FILE *infile, buffer_t outbuf)
{
	int ch;

	/* Read data from file to the buffer. */
	while ((ch = fgetc(infile)) != EOF)
		buffer_put(outbuf, ch & 0xff);
	
	buffer_put(outbuf, EOF);
}

int main()
{	
	//lendo os dados do arquivo de entrada
	//aki tah chamando direto pelo codigo, no trabalho msm 
	//pega pelo argumento passado
	//ou seja, fopen(argv[1]), responsabilidade do usuario passar um path correto
	buffer_t out = buffer_create(1024);
	unsigned i = buffer_get(out);

	//testando se buffer foi criado com valores nulos
	printf("%d\n",i);

	FILE *in = fopen("dummy.txt","r");
	lzw_readbytes(in,out);

	//testando se funcao deu certo
	i = buffer_get(out);
	printf("%d\n",i);

	//cria buffer q recebe dados de saida da compressao do arquivo de entrada
	buffer_t out_2 = buffer_create(1024);
	i = buffer_get(out_2);

	printf("%d\n",i);


	//emulando worker thread q le do buffer de entrada, comprime 
	//e coloca dados num outro buffer de saida
	lzw_compress(out,out_2);
	
	//testando se funcionou
	i = buffer_get(out_2);

	printf("%d\n",i);

	//escreve conteudo do buffer de saida no arquivo de saida
	FILE *output = fopen("dummy.lzw","w");

	if(output == NULL)
	{
		printf("deu zica\n");
		return 1;
	}

	lzw_writebytes(out_2,output);
	return 0;
}