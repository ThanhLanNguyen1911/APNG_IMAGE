#ifndef STB_IMAGE_IMPLEMENTATION //make sure stb image has already been included,if not,try including it
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif
#define APNGI_LOAD_PNG 0

short byte_order16(short value){
unsigned char *cp,buffer[2];
cp=(unsigned char*)&value;
buffer[0]=cp[0];
buffer[1]=cp[1];
//swap byte order
cp[0]=buffer[1];
cp[1]=buffer[0];
return value;
}

int byte_order32(int value){
unsigned char *cp,buffer[4];
cp=(unsigned char*)&value;
buffer[0]=cp[0];
buffer[1]=cp[1];
buffer[2]=cp[2];
buffer[3]=cp[3];
//swap byte order
cp[0]=buffer[3];
cp[1]=buffer[2];
cp[2]=buffer[1];
cp[3]=buffer[0];
return value;
}

unsigned char* apng_loaderM(char* data,int datasize,int *width,int *height,float *delay,int *frame,char *format,char flip,char load_alpha,float gamma){
int linecount=0,maxline,imgsize,chunksize=0,bufsize[1000];
char default_image=0,png_type=0,*cp,depth,bpp=1,*frame_start,*line_start;
unsigned char* img[1000],*ctab=0,*atab=0,*alphap=0,filter,left[3],above[3],upleft[3],triad[3];
#ifndef STB_IMAGE_IMPLEMENTATION //check stb image again
free(data);
return 0;
#endif
for(int i=12;i<datasize;++i){//skip magick number
if(data[i]=='I'&&data[i+1]=='H'&&data[i+2]=='D'&&data[i+3]=='R'){//ihdr chunk
i+=4;
*format=1;
*frame=0;
*delay=0;
*width=byte_order32(*(int*)&data[i]);//convert big endian to little endian
*height=byte_order32(*(int*)&data[i+4]);
float wdim=(float)*width/4;
if((wdim-(int)wdim)){//only support width that is divisible by 4
free(data);
return 0;
}
depth=data[i+8];
  if(depth!=8){//only support 8 bits color
  free(data);
  return 0;
  }
  if(data[i+9]==2){//RGB format
  *format=3;
  }else if(data[i+9]==6){//RGBA format
  *format=4;
  }else if(data[i+9]==3){//color lookup
  *format=3;
  }else if(data[i+9]==4){//gray scale + alpha
  *format=2;
  }
 maxline=width[0]*format[0];//row length in byte
 imgsize=width[0]*height[0]*format[0];//pixel size
 bpp=depth/8*format[0];//byte per pixel
}else if(png_type&&data[i]=='f'&&data[i+1]=='d'&&data[i+2]=='A'&&data[i+3]=='T'){//animated frame data
i+=10;//skip four bytes indice plus two bytes header
cp=&data[i];
//count data
chunksize=0;
while(png_type&&i<datasize){
if(data[i]=='f'&&data[i+1]=='c'&&data[i+2]=='T'&&data[i+3]=='L'){//stop at next frame control chunk
break;
}
if(data[i]=='I'&&data[i+1]=='E'&&data[i+2]=='N'&&data[i+3]=='D'){//stop at end chunk
break;
}
++chunksize;
++i;
}
//remove crc
chunksize-=4;
//decode data using stbi
img[*frame]=(unsigned char*)stbi_zlib_decode_malloc_guesssize_headerflag(cp,chunksize,imgsize+height[0],&bufsize[*frame],0);
   if(!img[*frame]){
   free(data);
   return 0;
   }
++frame[0];
}else if(!default_image&&data[i]=='I'&&data[i+1]=='D'&&data[i+2]=='A'&&data[i+3]=='T'){//default frame data
i+=6;//skip two bytes header
   if(png_type||APNGI_LOAD_PNG){
cp=&data[i];
//count data
chunksize=0;
while(!png_type&&(data[i]!='I'||data[i+1]!='E'||data[i+2]!='N'||data[i+3]!='D')){//PNG
++chunksize;
++i;
}
while(png_type&&(data[i]!='f'||data[i+1]!='c'||data[i+2]!='T'||data[i+3]!='L')){//APNG
++chunksize;
++i;
}
//remove crc
chunksize-=4;
//decode data
img[*frame]=(unsigned char*)stbi_zlib_decode_malloc_guesssize_headerflag(cp,chunksize,imgsize+height[0],&bufsize[*frame],0);
   if(!img[*frame]){
   free(data);
   return 0;
   }
++frame[0];
default_image=1;
   }else{
free(data);
return 0;
   }
}else if(data[i]=='P'&&data[i+1]=='L'&&data[i+2]=='T'&&data[i+3]=='E'){//load color table
ctab=(unsigned char*)&data[i+4];
maxline=*width;//reassign row length
}else if(data[i]=='t'&&data[i+1]=='R'&&data[i+2]=='N'&&data[i+3]=='S'){//load alpha table
if(load_alpha){
atab=(unsigned char*)&data[i+4];
*format=4;
 imgsize=width[0]*height[0]*4;//pixel size
 bpp=depth/8*4;//byte per pixel
}
}else if(data[i]=='a'&&data[i+1]=='c'&&data[i+2]=='T'&&data[i+3]=='L'){//animtion control chunk
if(!*(int*)&data[i+4]){//no frame available
free(data);
return 0;
}
png_type=1;//APNG DETECTED
}else if(png_type&&!delay[0]&&data[i]=='f'&&data[i+1]=='c'&&data[i+2]=='T'&&data[i+3]=='L'){//frame control chunk
alphap=(unsigned char*)&data[i];
*delay=(float)byte_order16(*(short*)&data[i+24])/byte_order16(*(short*)&data[i+26]);//get frame delay
}else if(gamma&&data[i]=='g'&&data[i+1]=='A'&&data[i+2]=='M'&&data[i+3]=='A'){//load gamma
gamma=byte_order32(*(int*)&data[i+4]);
gamma*=0.00001;
}
}
//allocate final image memory
unsigned char *finalimg=(unsigned char*)calloc(imgsize*frame[0],1);
cp=(char*)finalimg;
height[0]*=frame[0];
//convert data into readable

for(int f=0;f<*frame;++f){
linecount=0;
frame_start=cp;
 for(int i=0;i<bufsize[f];){
   if(linecount==maxline||!i){
   linecount=0;
   line_start=cp;
   filter=img[f][i];//filter type
   ++i;
   }
if(ctab){//palette color
cp[0]=ctab[img[f][i]*3];//red
cp[1]=ctab[img[f][i]*3+1];//green
cp[2]=ctab[img[f][i]*3+2];//blue
if(atab){
  if(atab+img[f][i]<alphap){
  cp[3]=*atab;//alpha
  }else{
  cp[3]=255;
  }
}
++linecount;
++i;
}else{//direct color
cp[0]=img[f][i];//red
if(*format>=3){
cp[1]=img[f][i+1];//green
cp[2]=img[f][i+2];//blue
if(*format==4){
cp[3]=img[f][i+3];//alpha
}
//apply kernel
if(filter==1){//sub
if(cp-bpp>=line_start){ cp[0]+=*(cp-bpp); }
if(cp-bpp+1>=line_start){ cp[1]+=*(cp-bpp+1); }
if(cp-bpp+2>=line_start){ cp[2]+=*(cp-bpp+2); }
}else if(filter==2){//up
if(cp-maxline>=frame_start){ cp[0]+=*(cp-maxline); }
if(cp-maxline+1>=frame_start){ cp[1]+=*(cp-maxline+1); }
if(cp-maxline+2>=frame_start){ cp[2]+=*(cp-maxline+2); }
}else if(filter>=3){
//left sample
left[0]=0;left[1]=0;left[2]=0;
if(cp-bpp>=line_start){ left[0]=*(cp-bpp); }
if(cp-bpp+1>=line_start){ left[1]=*(cp-bpp+1); }
if(cp-bpp+2>=line_start){ left[2]=*(cp-bpp+2); }
//above sample
above[0]=0;above[1]=0;above[2]=0;
if(cp-maxline>=frame_start){ above[0]=*(cp-maxline); }
if(cp-maxline+1>=frame_start){ above[1]=*(cp-maxline+1); }
if(cp-maxline+2>=frame_start){ above[2]=*(cp-maxline+2); }

if(filter==4){//paeth
//upper left sample
upleft[0]=0;upleft[1]=0;upleft[2]=0;
if(cp-maxline-bpp>=frame_start){ upleft[0]=*(cp-maxline-bpp); }
if(cp-maxline-bpp+1>=frame_start){ upleft[1]=*(cp-maxline-bpp+1); }
if(cp-maxline-bpp+2>=frame_start){ upleft[2]=*(cp-maxline-bpp+2); }
//estimation
 for(int p=0;p<3;++p){
   triad[0]=abs(above[p]-upleft[p]);
   triad[1]=abs(left[p]-upleft[p]);
   triad[2]=abs(left[p]+above[p]-upleft[p]*2);
   if(triad[0]<=triad[1]&&triad[0]<=triad[2]){
    cp[p]+=left[p];
   }else if(triad[1]<=triad[2]){
    cp[p]+=above[p];
   }else{
    cp[p]+=upleft[p];
   }
 }
}else{//average
cp[0]+=floor((left[0]+above[0])/2);
cp[1]+=floor((left[1]+above[1])/2);
cp[2]+=floor((left[2]+above[2])/2);
}
}
linecount+=format[0];
i+=format[0];
}
}
//gamma correct
  if(gamma&&gamma!=1){
cp[0]=255*std::max(0.0f,std::min(1.0f,(float)pow((float)cp[0]/255,gamma)));
cp[1]=255*std::max(0.0f,std::min(1.0f,(float)pow((float)cp[1]/255,gamma)));
cp[2]=255*std::max(0.0f,std::min(1.0f,(float)pow((float)cp[2]/255,gamma)));
  }
cp+=format[0];
  }
   free(img[f]);
}
//clear all data
free(data);
//vertical flip if wanted
if(flip){
stbi__vertical_flip(finalimg,*width,*height,*format);
}
return finalimg;
}

unsigned char* apng_loaderF(const char* filename,int *width,int *height,float *delay,int *frame,char *format,char flip,char load_alpha,float gamma){
FILE *apng_file=fopen(filename,"rb");
fseek(apng_file,0,SEEK_END);
int filesize=ftell(apng_file);
rewind(apng_file);
char *data=(char*)calloc(filesize,1);
fread(data,1,filesize,apng_file);
fclose(apng_file);
unsigned char *image=0;
if(apng_file&&(bool)(image=apng_loaderM(data,filesize,width,height,delay,frame,format,flip,load_alpha,gamma))&&image){
return image;
}
return 0;
}
