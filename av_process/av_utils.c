#include <stdio.h>
#include <stdlib.h>

int simplest_pcm16le_split(char *url)
{
	FILE *fp = fopen(url, "rb+");
	FILE *fp1 = fopen("output_l.pcm", "wb+");
	FILE *fp2 = fopen("output_r.pcm", "wb+");

	unsigned char *sample = (unsigned char *)malloc(4);

	while (!feof(fp))
	{
		fread(sample, 1, 4, fp);
		//L
		fwrite(sample, 1, 2, fp1);
		//R
		fwrite(sample + 2, 1, 2, fp2);
	}
	free(sample);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	return 0;
}

int simplest_pcm16le_halfvolumeleft(char *url)
{
	FILE *fp = fopen(url, "rb+");
	FILE *fp1 = fopen("output_halfleft.pcm", "wb+");

	int cnt = 0;

	unsigned char *sample = (unsigned char *)malloc(4);

	while (!feof(fp))
	{
		short *l_s = NULL;
		fread(sample, 1, 4, fp);

		l_s = (short *)sample;
		*l_s = *l_s * 2;
		if (cnt == 100000)
			printf("%d %d", *sample, *(sample + 2));
		// break;
		//L
		fwrite(sample, 1, 2, fp1);
		//R
		// fwrite(sample+2,1,2,fp1);

		cnt++;
	}
	printf("Sample Cnt:%d\n", cnt);

	free(sample);
	fclose(fp);
	fclose(fp1);
	return 0;
}

int main()
{
	char *url = "/home/zxfeng/software/ffmpeg/ffmpeg_test/video/out.pcm";
	simplest_pcm16le_halfvolumeleft(url);
	return 0;
}
