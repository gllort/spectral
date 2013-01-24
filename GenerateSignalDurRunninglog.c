#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>


typedef struct {
	long long int *t;
	long long int *delta;
	long long int *value;
} signal;



void Sum(signal *result, long long int *c,  signal a, int i, signal b, int j) {

   int k, p;
   
   k=0;
   p=0;
   
   *result[0].t = 0;
   *result[0].value = 0;
   result[0].t++; result[0].value++; result[0].delta++;
   
   /*for(k=0; k<i; k++) {
   	printf("a %lld, %lld, %d\n", *(a.t), *(a.delta), *(a.value));
   	a.t++; a.delta++; a.value++;
   }
   
   a.t = a.t - i;
   a.delta = a.delta - i;
   a.value = a.value - i;
      
   
   for(k=0; k<j; k++) {
   	printf("b %lld, %lld, %d\n", *(b.t), *(b.delta), *(b.value));
   	b.t++; b.delta++; b.value++;
   }
   
   b.t = b.t - j;
   b.delta = b.delta - j;
   b.value = b.value - j;*/
   
    
   k=0;
   p=0;
   *c=1;

   //printf("i=%d, j=%d, k=%d, p=%d, c=%lld\n", i, j, k, p, *c);
   
   
   while((k<i-1) || (p<j-1)) {
	if((k<i-1) && (p<j-1)) {
   		if( *(a.t+1) < *(b.t+1) ) { 
			
			*result[0].t = *(a.t+1);
			*(result[0].delta-1)= *(a.t+1) - *(result[0].t-1);
			*result[0].value = *(a.value+1) + *(b.value);
			a.t++; a.delta++; a.value++;
			k++;
			if(*(result[0].delta-1)<0) {
		                //printf("1Warning!!!!!\n");
                		//printf("i=%d, j=%d, k=%d, p=%d, c=%lld\n", i, j, k, p, *c);
        		}
		}
		else { 
			*result[0].t = *(b.t+1);
			*(result[0].delta-1)= *(b.t+1) - *(result[0].t - 1);
			*result[0].value = *(b.value+1) + *(a.value);
			b.t++; b.delta++; b.value++;
			p++;
			if(*(result[0].delta-1)<0) {
		                //printf("2Warning!!!!!\n");
                		//printf("i=%d, j=%d, k=%d, p=%d, c=%lld\n", i, j, k, p, *c);
        		}
		}
	}
	if((k==i-1) || (i==0)) {
		*result[0].t = *(b.t+1);
                *(result[0].delta-1)= *(b.t+1) - *(result[0].t - 1);
                *result[0].value = *(b.value+1); 
                b.t++; b.delta++; b.value++;
                p++;
	}
	if((p==j-1) || (j==0)) {
		*result[0].t = *(a.t+1);
                *(result[0].delta-1)= *(a.t+1) - *(result[0].t - 1);
                *result[0].value = *(a.value);
                a.t++; a.delta++; a.value++;
                k++;
	}
	/*printf("a %lld, %lld, %d\n", *(a.t), *(a.delta), *(a.value));
	printf("b %lld, %lld, %d\n", *(b.t), *(b.delta), *(b.value));
	printf("c %lld, %lld, %d\n", *(result[0].t-1), *(result[0].delta-1),
	*(result[0].value-1));*/
	result[0].t++;
	result[0].delta++;
	result[0].value++;
	(*c)++;
   }
   //printf("i=%d, j=%d, k=%d, p=%d, c=%lld\n", i, j, k, p, *c);
    
}
	
   
	
   	
   
   	


void GenerateSignal(char *dades, signal *sig, int p, long long int *count)
{

  FILE *fp;
  char s[8000];
  long long int t1, j, tant, t0, thread, t1bis, tantbis;
  long long int status, statustype;
  long long int sum;
  int cale, i, firstnumber;
  char first[2];
  //signal *tmp;
  //long long int value, t0bis;
  
  
  
  for(i = 0; i<p; i++) {
  	count[i]=0;
  }
  
  cale=0;
  firstnumber=1;
 

  fp=fopen(dades, "r");
  if(fp==NULL) {
      printf("File not open\n");
        exit(1);
  }

  j=0;

  while (fgets(s, 8000, fp)!=NULL) {
        j++;  
  }
    
  fseek(fp, 0L, SEEK_SET);


  while((cale ==0) && (fgets(s, 8000, fp)!=NULL)) {
      sscanf(s, "%1s", first);
      i=atoi(first);
      if(i==1 || i==2 || i==3 || i==4) {
            cale=1;
      }
  }
  fseek(fp, -1L, SEEK_CUR);
  fgets(s, 100, fp);

  sum=0;

  sscanf(s, "%lld", &status);
  sscanf(s, "%*[^:]:%lld", &thread);
  sscanf(s, "%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%lld", &t0);
  sscanf(s, "%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%lld", &t1);
  sscanf(s, "%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%lld", &statustype);
  /*printf("%s\n", s);
  printf("%lld %lld %lld %lld\n", status, t0, t1, statustype);*/
  t1=0;
  t1bis=0;
  thread--;
  if(status==1) {
  	if (statustype==1) {
		*sig[thread].t=0;
		*sig[thread].delta=t0;
		*sig[thread].value=0;
		/*printf("%lld %lld %d\n", *sig[thread].t, *sig[thread].delta, *sig[thread].value);*/
		sig[thread].t++;
		sig[thread].delta++;
		sig[thread].value++;
		*sig[thread].t=t0;
                *sig[thread].delta=t1-t0;
                *sig[thread].value=t1-t0;
		/*printf("%lld %lld %d\n", *sig[thread].t, *sig[thread].delta, *sig[thread].value);*/
		sig[thread].t++;
		sig[thread].delta++;
                sig[thread].value++;
		count[thread]=count[thread]+2;
		/*if(thread==p) {
			t0bis=t0;
			t1bis=t1;
			fprintf(gp, "%lld %lld %d\n", 0, t0bis, 0);
			fprintf(gp, "%lld %lld %d\n", t0bis, t1bis-t0bis, 1);
			tantbis=t1bis;
			tant=t1;
			}*/
	}
  }
  tant=t1;
  tantbis=t1bis;
  while(fgets(s, 100, fp)!=NULL) {
	sscanf(s, "%lld", &status);
        sscanf(s, "%*[^:]:%lld", &thread);
        sscanf(s, "%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%lld", &t0);
        sscanf(s, "%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%lld", &t1);
        sscanf(s, "%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%lld", &statustype);
	/*printf("%s\n", s);
        printf("%lld %lld %lld %lld \n", status, t0, t1, statustype);*/
	thread--;
	if(status==1) {
                if (statustype==1) {
			if(count[thread]==0) {
				*sig[thread].t=0;
				*sig[thread].delta=t0;
				*sig[thread].value=0;
			}
			else {
				*sig[thread].t=*(sig[thread].t-1)+*(sig[thread].delta-1);
	                	*sig[thread].delta=t0-(*(sig[thread].t-1)+*(sig[thread].delta-1));
				if(*sig[thread].delta < 0) {
					//printf("Warning thread=%lld t0=%lld \n", thread, t0);
				}
        	        	*sig[thread].value=0;
			}
			/*printf("%lld %lld %d\n", *sig[thread].t, *sig[thread].delta, *sig[thread].value);*/
                	sig[thread].t++;
                	sig[thread].delta++;
                	sig[thread].value++;
			if(t1>t0) {
                		*sig[thread].t=t0;
                		*sig[thread].delta=t1-t0;
                		*sig[thread].value=t1-t0;
                		sig[thread].t++;
                		sig[thread].delta++;
                		sig[thread].value++;
				count[thread]=count[thread]+2;
			} 
			else {
				count[thread]=count[thread]+1;
				//printf("t0=%lld t1=%lld\n", t0, t1);
			}
			/*if(thread==p) {
				printf("%lld %lld %d\n ", *(sig[thread].t-1),
				*(sig[thread].delta-1), *(sig[thread].value-1));
				t0bis=t0;
				t1bis=t1;
               	        	fprintf(gp, "%lld %lld %d\n", tantbis, t0bis-tantbis, 0);
                       		fprintf(gp, "%lld %lld %d\n", t0bis, t1bis-t0bis, 1);
				tantbis=t1;
				tant=t1;
				}*/
		}
       	}

  }
    
  fclose(fp);

}

int main(int argc, char *argv[]) {

  FILE *fp;
  int p, i;
  char s[100];
  signal *sig, *a;
  int j;
  long long int num, *count;


  if(argc!=3) {
      printf("Usage: %s <input file> <output file>\n", argv[0]);
        exit(0);
  }

  fp=fopen(argv[1], "r");
  fgets(s, 100, fp);
  sscanf(s, "%*s %*[^:]:%*[^:]:%*[^:]:%d", &p);
  //printf("threads=%d\n", p);

  fseek(fp, 0L, SEEK_SET);

 
  j=0;
 

  while (fgets(s, 100, fp)!=NULL) {
        j++;
  }

  fclose(fp);

  //printf("%d\n", j);
  

  sig = (signal *)malloc(sizeof(signal)*p);
  count = (long long int *)malloc(sizeof(long long int) * p);
  a = (signal *)malloc(sizeof(signal)*1);
  a[0].t = (long long int *)malloc(sizeof(long long int) *3*j);
  a[0].delta = (long long int *)malloc(sizeof(long long int) *3*j);
  a[0].value = (long long int *)malloc(sizeof(long long int) *3*j); 

  
  sig[0].t = (long long int *)malloc(sizeof(long long int)*3*j);
  sig[0].delta = (long long int *)malloc(sizeof(long long int)*3*j);
  sig[0].value = (long long int *)malloc(sizeof(long long int)*3*j);

  for (i = 1; i < p; i++) {
		sig[i].t = (long long int *)malloc(sizeof(long long int) * j/2);
		sig[i].delta = (long long int *)malloc(sizeof(long long int) * j/2);
		sig[i].value = (long long int *)malloc(sizeof(long long int) * j/2);
  }
  
  
  GenerateSignal(argv[1], sig, p-1, count);

  
  for(i = 0; i<p; i++) {
  	sig[i].t=sig[i].t-count[i];
	sig[i].delta=sig[i].delta-count[i];
	sig[i].value=sig[i].value-count[i];
  }


  /*for(i=0; i<count[p]; i++) {
	printf("%lld %lld %d\n", *sig[p].t, *sig[p].delta, *sig[p].value);
	sig[p].t++;
	sig[p].delta++;
	sig[p].value++;
	}
  sig[p].t=sig[p].t-count[p];
  sig[p].delta=sig[p].delta-count[p];
  sig[p].value=sig[p].value-count[p];*/


  for(i=1; i<p; i++) {
  	
	//printf("zz=%d\n", i);
  
  	Sum(a, &num, sig[0], count[0], sig[i], count[i]);

	//printf("num=%lld\n", num);	
	
	(*a).t=(*a).t-num;
  	(*a).delta=(*a).delta-num;
  	(*a).value=(*a).value-num;
  
  
  	/*sig[0].t=sig[0].t-count[0];
  	sig[0].delta=sig[0].delta-count[0];
  	sig[0].value=sig[0].value-count[0];*/
	
	
  	for(j=0; j<num-1; j++) {
		/*if (i>245 & i<256) {
	  		printf("d %lld, %lld, %d\n", *((*a).t), *((*a).delta), *((*a).value));
		}*/
		*sig[0].t=*((*a).t);
		*sig[0].delta=*((*a).delta);
		*sig[0].value=*((*a).value);
   		(*a).value++; (*a).delta++; (*a).t++;
		sig[0].t++; sig[0].delta++; sig[0].value++;
  	}
  
  	(*a).t=(*a).t-num+1;
  	(*a).delta=(*a).delta-num+1;
  	(*a).value=(*a).value-num+1;
  
  	sig[0].t=sig[0].t-num+1;
  	sig[0].delta=sig[0].delta-num+1;
  	sig[0].value=sig[0].value-num+1;
    
  
  	count[0]=num;
  
  }
  fp=fopen(argv[2], "w");
  for(j=0; j<count[0]-2; j++) {
  	fprintf(fp, "%lld %lld %lf\n", sig[0].t[j], sig[0].delta[j], log(1.0*sig[0].value[j]+1));
	/*fprintf(fp, "%lld %lld %lf\n", sig[0].t[j], sig[0].delta[j], (1.0*sig[0].value[j])/(1000000000.0));*/
  	/*sig[0].value++; sig[0].delta++; sig[0].t++;*/
  
  }
  fclose(fp);

  return 0;
}


