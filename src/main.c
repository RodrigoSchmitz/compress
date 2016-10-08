#include <buffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <global.h>
#include <util.h>
#include <dictionary.h>
#include <errno.h>
#include <string.h>	
#include <pthread.h>
#include <semaphore.h>

#define RADIX 256 /* Radix of input data. */
#define WIDTH  12 /* Width of code word.  */
#define BUFFSIZE 1024


/*static void lzw_writebytes(buffer_t inbuf, FILE *outfile)
{
	int ch;
	
	 Read data from file to the buffer. 
	while ((ch = buffer_get(inbuf)) != EOF)
		fputc(ch, outfile);
}
*/
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

static void lzw_readbits(FILE *in, buffer_t out)
{
	int bits;   /* Working bits. */
	unsigned n; /* Current bit.  */
	int buf;    /* Buffer.       */
	
	n = 0;
	buf = 0;
	
	/*
	 * Read data from input file
	 * and write to output buffer.
	 */
	while ((bits = fgetc(in)) != EOF)
	{	
		buf = buf << 8;
		buf |= bits & 0xff;
		n += 8;
				
		/* Flush bytes. */
		while (n >= WIDTH)
		{
			buffer_put(out, (buf >> (n - WIDTH)) & ((1 << WIDTH) - 1));
			n -= WIDTH;
		}
	}
			
	buffer_put(out, EOF);
}
//===============================================================================

sem_t itensproducao,itensconsumo;
buffer_t out;
buffer_t out_2;
FILE *in;
pthread_mutex_t mutex;
int produto = 5;

void *consumir(void *arg)
{
	int i = 0;

	while(i < 10)
	{
		sem_wait(&itensconsumo);
		pthread_mutex_lock(&mutex);
		printf("consumindo item do buffer: %u\n",buffer_get(out));
		pthread_mutex_unlock(&mutex);
		sem_post(&itensproducao);
		i++;
	}

	return NULL;
}

void *produzir(void *arg)
{
	int i = 0;

	while(i < 10)
	{
		sem_wait(&itensproducao);
		pthread_mutex_lock(&mutex);
		//int bit = fgetc(in) != EOF ? fgetc(in) : -1;
		//printf("bit: %d\n",bit);
		int bit = rand()%50; 		
		printf("produzindo item no buffer: %d\n",bit);
		buffer_put(out,bit);
		pthread_mutex_unlock(&mutex);
		sem_post(&itensconsumo);
		i++;
	}

	return NULL;
}

int main()
{
	out = buffer_create(BUFFSIZE);		//buffer q abriga dados do arquivo de entrada
	sem_init(&itensconsumo,0,0);
	sem_init(&itensproducao,0,BUFFSIZE);
	in = fopen("dummytext.txt","r");
	pthread_t consumidor,produtor;
	pthread_create(&produtor,NULL,produzir,NULL);
	pthread_create(&consumidor,NULL,consumir,NULL);
	pthread_join(consumidor,NULL);
	pthread_join(produtor,NULL);
	return 0;
}