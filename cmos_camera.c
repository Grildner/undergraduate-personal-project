#include  "stc15f2k60s2.h"		  
#include  "def.h"//宏定义
#include  "delay.h"//延时
#include  "tft.h"//TFT IC底层驱动
#include  "gui.h"
#include  "xpt2046.h"
#include  "spi.h"
#include  "key.h"
#include  "sd.h"
#include  "pff.h"//文件系统调用.h加载
#include  "flash.h"
#include  "sram.h"
#include  "ov7670.h"
#include  "string.h"
#define   Unused_Variant(x) ((void)(x)) //宏通知编译器，局部变量无效
FATFS fatfs;u8 tbuf[512];u8 pic_width;u16 pic_height;
int n = 0;                   
int p = 0;                       
u16 u=3000;
u16 i=0;
int en[30] = 
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
void delay(unsigned int m)			
{
	 int  a, b;
	 for(a=0;a<5000;a++)
	 {for(b=0;b<m;b++);}  
}
void IO_init(void)
{
P0M0 = 0X00;
P0M1 = 0X00;
P1M0 = 0X00;
P1M1 = 0X00;
P2M0 = 0X00;
P2M1 = 0X00;
P3M0 = 0X00;
P3M1 = 0X00;
P4M0 = 0X00;
P4M1 = 0X00;  
}
void GetTouchScreenPos(struct TFT_Pointer * sp)
{
		u16 a,b, flag;u8  ax[8];u16 x1,y1;u16 x2,y2;
		u8 i2t[8];
//读24c02中 触摸校准参数 临时转换调用数组
//触摸屏校准参数
		struct T_i T_i2c={656,883,-13,-12,};
//读取AD值并转换为X Y坐标
#define ERR_RANGE 5//误差范围 
	  if(AD7843_isClick==0)
		{	 
			delay1ms(1);
			if(AD7843_isClick==0)
			{
				Unused_Variant(i2t);
				LCD_CS=1;			//xpt的片选线在tft上,防止触摸工作时影响tft	,这里关掉TFT使能
				AD7843_CS=0; 	//开片选
				SPI_Speed(2);	
//降低 SPI通讯速度 使触摸芯片放回数据更稳定
/*这里采用16时钟周期采集。因为，此触摸功能采用的是SPI总线。而SPI功能是只能8位传输 ，XPT2046的AD分辨率为12位 ，需要读两次。
根据XPT2046手册中，16时钟周期-时序图，可以看出发送采集数据 ，接收一次SPI数据后，再发送空功能的SPI数据 ，就会把剩下的部分接收到；
这样先接收的，是低字节数据 ，第二次接收的是高字节数据 。移位后，便是12位的AD值。 */  
				ax[0]=SPI_SendByte(0x90); //送控制字10010000即用差分方式读X坐标，舍弃读取的数据
				ax[0]=SPI_SendByte(0x00); //发送任意数据（最高位不能为1，和2046命令冲突），接收X高字节
				ax[1]=SPI_SendByte(0xD0); //送控制字11010000即用差分方式读Y坐标，接收X低字节
				ax[2]=SPI_SendByte(0x00); //发送任意数据（同上），接收Y高字节
				ax[3]=SPI_SendByte(0x90); //送控制字10010000 （第二次）读X坐标，接收Y低字节
				ax[4]=SPI_SendByte(0x00); //发送任意数据（同上），接收X高字节
				ax[5]=SPI_SendByte(0xD0); //送控制字11010000  （第二次）读Y坐标，接收X低字节
				ax[6]=SPI_SendByte(0x00); //发送任意数据（同上)，接收Y高字节
				ax[7]=SPI_SendByte(0x90); //送控制字10010000  （第三次）读X坐标，接收Y低字节   
                //提取两次采集值
				y1=(ax[0]<<5)|(ax[1]>>3);
				y2=(ax[4]<<5)|(ax[5]>>3);
				x1=(ax[2]<<5)|(ax[3]>>3);
				x2=(ax[6]<<5)|(ax[7]>>3);

if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-ERR_RANGE内
						&&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
				{
					flag=1;			    //打开标志位
					a=(x1+x2)/2;
					b=(y1+y2)/2;
				}
				else flag=0;
				SPI_Speed(0);		    //调整SPI速度为最高
				AD7843_CS=1; 		    //关片选
				LCD_CS=0;
			}
		} 
/* 触摸屏计算公式lcdx=xa*tpx+xb;lcdy=ya*tpy+yb;
   lcdx,lcdy为屏坐标;tpx,tpy为触屏板值;xa,ya为比例因子;xb,yb为偏移量
   计算方法: 
   在屏幕上指定lcdx,lcdy位置画出十字图形,分别画在屏幕上的4个角位置
   用触摸笔分别点击,得到其中的触摸值
   根据上面的公式计算xa,ya、xb,yb 这样就能使得触摸板和屏幕校准*/
	//无校准功能  
	//result.x=0.065894*a-16;
//将得到的AD值带入公式 计算lcd屏的x y坐标 
		//result.y=0.084031*b-14;		
		//加了校准功能  
		 sp->x = ((float)T_i2c.xi/10000)*a+T_i2c.a;			
//将得到的AD值带入公式 计算lcd屏的x y坐标 
		 sp->y = ((float)T_i2c.yi/10000)*b+T_i2c.b;
		 sp->flag = flag;
}
//显示16位色BMP图片.图片显示在屏幕的正中位置
//进入bmp首扇区后,前4个字节为文件大小:  
u8 Show_Bmp(const char *path)
{
	FRESULT res; 
	u16 br,y=0,zy,height,	      //width,height图片的初始左边
			y1,i1,tmp;		         //tmp 16位解码变量
	u8  x=0,zx,width,x1,
      rgb=0, Bmpcolor;
	res=pf_open(path);		 
  //打开指定路径文件,这一步可以打开任何路径下的文件,注意它的功能就是打开文件，不是文件夹
  //读文件内数据的前提是必须要打开这个文件
	if(res == FR_OK)
	{
			pf_read(tbuf, 54, &br);		
 //取前54字节,前54字节含有bmp文件大小,文件长宽尺度.像素值等数据信息   
			if(br!=54) return 1;		         //提取出错
//实际宽和高都是用4个字节表示的，但图片的大小不能超过屏的尺寸，故这里只用一个字节表示宽度,两个字节表示高度
			width  = tbuf[18];				    //计算图片宽度	 
			height = (tbuf[23]<<8)+tbuf[22];	//计算图片高度
			pic_width  = width;
			pic_height = height;
			Bmpcolor=tbuf[28]/8;			    //获取颜色深度。一般是16位、24位、32位  
//number(30,280,Bmpcolor,Red,White);            //将小于屏幕尺寸的图片放到屏幕正中间，顶头显示
			if(width<239)   zx=(240-width)/2;         else zx=0;
			if(height<299)	zy=(320-height);          else zy=0;	
			x1=zx; y1=zy-80;
//赋值计算后的值，改变y1值，使显示的正向图像顶头显示 
			LCD_scan(3);	  
//BMP图片解码的扫描方式为：从左到右、从下到上，否则显示的图片上下颠倒
			Address_set(x1,y1,x1+width-1,y1+height-1);
//设置显示范围 先扫横行 再扫纵行
			LCD_RS=1;    
//写数据线拉高。为提高写入速度，主循环前拉高
			while(1)                             //一直到最后一簇
			{
		  		 pf_read(tbuf, 512, &br);
//从54字节后位置读取512个字节到缓存区  
					 for(i1=0;i1<512;i1++)
						{
							if(Bmpcolor==2)		 //16位BMP
							{			
								switch(rgb)		//555转565格式
								{
									case 0:
						       tmp=((u16)tbuf[i1]&0x1f);	                                
//R
										tmp+=(((u16)tbuf[i1])&0xe0)<<1;	                                             //G
									break;

									case 1:
										tmp+=(u16)tbuf[i1]<<9;				                                         //B
									break;		     		 
								} 
							}
							else if(Bmpcolor==3)
//24位BMP解码 RGB分别占8个字节
							{
								switch(rgb)
								{
									case 0:
										tmp=tbuf[i1]>>3;				        	                                      //B
									break;

									case 1:
										tmp+=((u16)tbuf[i1]<<3)&0x07e0;	                                        //G
									break;

									case 2:
										tmp+=((u16)tbuf[i1]<<8)&0xf800;	                                        //R
									break; 
								}
							}			
							rgb++;
							if(rgb==Bmpcolor)
							{ 
								P2=tmp>>8;								
//为了提高显示速度 直接调用IO口本身
								P0=tmp;									            //void Lcd_Write_Data(u16 Data)函数的分解
								tmp=0;
								rgb=0;
								LCD_WR=0;			//开始写入
								LCD_WR=1;			  	       
								x1++;							             //横向自加 加满一行 横向清零 纵向自加 直到扫完整体图片
								if(x1==width+zx)                  
								{	 
									y1--;
									x1=zx;
									if(y1==zy-height-80)    
//改变显示的判断条件，显示图像大小为240*240 
									{								
//恢复正常扫描方式
										LCD_scan(1);
										return 0;    //显示完成	
									}
								}
							}
						} 
			}
  }
	return 1;                             //出错
}		    
u8 Save_Bmp(const char *path)
{
	FRESULT res;
	u16 br,i,j,m=0,n=319,color;         //改变n，使存图满足正向显示图像的240*240尺寸 
	sram(1);							             
//开启外部内存。由于文件路径在外部SRAM中 所以这里要开启外部SRAM才能调用路径
	res=pf_open(path);		            
 //打开指定路径文件，这一步可以打开任何路径下的文件。注意它的功能就是打开文件，不是文件夹
	if(res == FR_OK)
	{
			led = 0; //存盘指示灯(P3.5)亮
			delay(200);led = 1;delay(100);led = 0;
			pf_read(tbuf,54,&br);				 	
//提取BMP图片前54字节图片信息
			pf_open(path);		 						
//重新打开路径,将扇区指向图片首数据位置
			sram(0);							 			 	
//关闭外部内存，开启液晶片选
			for(i=27;i<256;i++)				 		
//提取512个字节到tbuf中,即256个颜色点
      {
					color=LCD_readpoint(m,n);	
//提取LCD每个像元的颜色
					color=((color>>1)&0x7fe0)+(color&0x001f);	
//将提取的565格式转换为555格式
					tbuf[i*2]=color;				 	
//存入时低字节在前
					tbuf[i*2+1]=(color>>8);
					m++;
      }		
			pf_write(tbuf,512,&br);			 	
//向TF卡内写入512个字节（1个扇区）
      for(j=0;j<254;j++)
		 {
	     for(i=0;i<256;i++)				 		
//提取512个字节到tbuf中.即256个颜色点
	     {
					color=LCD_readpoint(m,n);	
//符合摄像头摄像效果，提取颜色
					color=((color>>1)&0x7fe0)+(color&0x001f); 
//将提取的565格式转换为555格式

	        tbuf[i*2]=color;				 	    //存入时低字节在前
					tbuf[i*2+1]=(color>>8);
					m++;
					if(m==240)
					{
						m=0;
						n--;
//这里不用判断m ，因为循环固定，直接会跳出 
					}
	     }	  
	     pf_write(tbuf,512,&br);			 
//向TF卡内写入512个字节		 
		 }
		SD_DisSelect();
//取消TF卡片选，在写入函数里加取消片选，会有影响，所以在最后写入完成加取消片选
		delay1ms(20);                      //保持显示延时
		led = 1;                          //存盘指示灯(P3.5)灭
		return 0;                         //写入成功
	}
	else
	{
		return 1;                          //错误
	}
}
void main()
{ 
	u32  j;					  	    	   //临时变量
	u8  sign=0;			  		        //初始标志
	struct TFT_Pointer sp;             //定义触摸变量结构体
	struct TFT_Pointer tp;             //用来检查触摸屏是否有键按下
	char *fname[30]=
{"/ov76/M1.bmp","/ov76/M2.bmp","/ov76/M3.bmp","/ov76/M4.bmp","/ov76/M5.bmp","/ov76/M6.bmp","/ov76/M7.bmp","/ov76/M8.bmp","/ov76/M9.bmp","/ov76/M10.bmp","/ov76/M11.bmp","/ov76/M12.bmp","/ov76/M13.bmp","/ov76/M14.bmp","/ov76/M15.bmp","/ov76/M16.bmp","/ov76/M17.bmp","/ov76/M18.bmp","/ov76/M19.bmp","/ov76/M20.bmp","/ov76/M21.bmp","/ov76/M22.bmp","/ov76/M23.bmp","/ov76/M24.bmp","/ov76/M25.bmp","/ov76/M26.bmp","/ov76/M27.bmp","/ov76/M28.bmp","/ov76/M29.bmp","/ov76/M30.bmp"}; 
	int x=0, y=300;	           //显示字符串的坐标
	/*初始化*/
  SP=0X80;				      	// 调整堆栈指向
  IO_init();				  	   // 对IAP15W4K61S4  IO口初始化
  Lcd_Init();                 // TFT液晶初始化
  Init_SPI(); 			      // SPI初始化
  GUI_Clear(White);			  // 白色清屏 
  SD_Init();			         // SD卡初始化
  KEY_Init();	
  pf_mount(&fatfs);	         //初始化petit FATFS文件系统  并提取tf卡相应数据
  //这句非常重要，它是使用所有Petit Fatfs文件系统功能的前提
  mem_init();				  		//外部SRAM初始化
if (SD_Init()) 			      //检测SD卡，初始化
	      {
		       GUI_sprintf_hzstr16x(90,20,"SD卡初始化失败.",Red, White); 
		       return;
	      }			      
else
	      {GUI_sprintf_hzstr16x(90,20,"已初始化SD卡.",Black, White);}
	 P1M0=0X00;//P1口仅输入
	 P1M1=0Xff;		
	/* 开启中断目的为判断VSYNC（帧数据判断）：当有触发的时候为来一帧
	   这时开始往摄像头FIFO缓存芯片AL422B灌入一帧数据。
		 当来第二帧时说明数据灌入完成，此时提取数据进行显示。 */
  IT0=1;			                    //边沿触发   
  EX0=1;                            //外部中断0   P3.2口    
  EA =1;                           //开总中断 
  GUI_Clear(Black);                //清空显示屏
  LCD_scan(1);                     //屏幕扫描：从上到下，从右到左
  if( Ov7670_init()) 
  {
   GUI_sprintf_hzstr16x(40,150,"Срождеством",Yellow,Red);
	 GUI_sprintf_hzstr16x(40,170,"Христовым",Yellow,Red); //开机欢迎语
   delay1ms(200);
   EX0=0;			                   //关闭外部中断0
   EA=0;			                   //关闭总中断
   sign=1;
  }
	 GUI_sprintf_hzstr16x(60,242,"な", Red,Black);
	 GUI_sprintf_hzstr16x(60,267,"ん", Red,Black);
	 GUI_sprintf_hzstr16x(60,292,"で？", Red,Black);//开机文字
	 delay(500);		
   GUI_Clear(Black); 
	 GUI_box(20,280,240,320,Black);     //界面布局 
   GUI_sprintf_hzstr16x(50,272,"Archive", White,Black);   
//相片
   GUI_arc(180,280,12,White,1);	
   GUI_arc(180,280,14,White,0);
   GUI_arc(180,280,15,White,0);         //拍照按钮
   if(sign==1) return;
	 FIFO_OE=0;//相机使能
	 OV7670_Window_Set(10,176,240,240);	//设置窗口大小：240*240
/*==================================================================================================================================*/ 
	while(1)                                         
 {
	 FIFO_OE=0; 
//打开相机摄像头.为无触摸操作时，不断刷新屏幕
	 LCD_scan(4);Address_set(0,80,239,319);
	 if(cur_status==2)			      
         { 
           FIFO_RRST=0;FIFO_RCLK=0;FIFO_RCLK=1;FIFO_RCLK=0;FIFO_RRST=1;FIFO_RCLK=1;LCD_RS=1;		   
	         for(j=0;j<57600;j++)	   
	         {		 
						 FIFO_RCLK=0;P2=P1;FIFO_RCLK=1;FIFO_RCLK=0;P0=P1;FIFO_RCLK=1;LCD_WR=0;LCD_WR=1;		
	         }  
						EX0 = 1;cur_status=0;			              
         } 
	 FIFO_OE=1;GetTouchScreenPos(&tp);      //获取屏幕触控区域
	 if ((tp.x>150) && (tp.x<210) && (tp.y>250) && (tp.y<310))
//拍照
	 {		
				LCD_scan(1);GUI_arc(180,280,12,Black,1);
				delay(200);GUI_arc(180,280,12,White,1);	tp.flag = 0;                                            // 重置触控标记
				LCD_scan(4);
				if(Save_Bmp(fname[n])==0)    //保存已拍照片 
 				 {
					LCD_scan(1);GUI_sprintf_hzstr16x(70, 300, "Saved!", White, Black); 
					delay(500);GUI_sprintf_hzstr16x(70, 300, "Saved!", Black, Black); 	
 				 }
				else
				{
					LCD_scan(1);
					GUI_sprintf_hzstr16x(70, 300, "Shit!", Red, Black); 
//保存失败 
					delay(500);GUI_sprintf_hzstr16x(70, 300, "Shit!", Black, Black);
				}
				delay(200);
	 }
	 if ((tp.x>15) && (tp.x<60) && (tp.y>260) && (tp.y<310))  
// 相册
	 {
		 LCD_scan(1);
		 GUI_sprintf_hzstr16x(20,272,"Archive", Black,Purple); 
		 delay(100);GUI_sprintf_hzstr16x(20,272,"Archive", White,Black);
		 p = n;              // previous上一张照片 next下一张照片
		 x=0; y=300;	
		 GUI_sprintf_hzstr16x(0,272,"Previous", White,Black); 
		 GUI_sprintf_hzstr16x(100,272,"Next", White,Black); 
		 GUI_sprintf_hzstr16x(180,272,"Camera", White,Black);  
		 p = p-1;            //用来显示上一张拍摄的照片 
		 if(p<0) p=29;       //防止溢出 
		 if(p>29) p = 0;	delay(200);FIFO_OE = 1; GetTouchScreenPos(&tp);		 
		 L1:	if(p<0) p=29;  //Label1 相册循环位置
		 if(Show_Bmp(fname[p])==0)        //显示照片 
			{		
				while(1)                   //相册模式主循环 
					{
				GetTouchScreenPos(&tp);   //碰撞检测
				if(p<0) p=29;
				if(p>29) p = 0;
				if ((tp.x>150) && (tp.x<210) && (tp.y>250) && (tp.y<310))//返回相机模式 
				{
					GUI_sprintf_hzstr16x(180,272,"Camera", Black,Purple);                                    //触屏提示 
					delay(100);
					GUI_sprintf_hzstr16x(180,272,"Camera", White,Black);
					GUI_Clear(Black);
					GUI_box(20,280,240,320,Black);
					GUI_sprintf_hzstr16x(50,272,"Archive", White,Black); 
					break; 
				}
				if ((tp.x>80) && (tp.x<140) && (tp.y>240) && (tp.y<280))//Next
					{
						GUI_sprintf_hzstr16x(100,272,"Next", Black,Purple);                                  //触屏提示 
						delay(100);
						GUI_sprintf_hzstr16x(100,272,"Next", White,Black); 
					p = p+1;                   //Next
					if(p>29) p = 0;            //避免溢出 
					goto L1; 
					}
				if ((tp.x>0) && (tp.x<60) && (tp.y>250) && (tp.y<310))//Prvious
				{
						GUI_sprintf_hzstr16x(0,272,"Previous", Black,Purple);
						delay(100);
					  GUI_sprintf_hzstr16x(0,272,"Previous", White,Black);
					p = p - 1; //Previous
					if(p<0) p=29; 
					goto L1;
				}
			  }
				}			
		 }
 }
}
