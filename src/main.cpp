//攀登：土壤传感器
//供电：12V
//通信：RS485-TTL（数据格式见文件截图）

#include <Arduino.h>   

/*函数声明*/
uint16_t CRC16(const uint8_t *data, uint16_t length);
bool checkCRC(const uint8_t *data, uint16_t len);
void parseModbusData(const uint8_t *data, uint16_t len);


/*************询问命令***************/

//土壤ph值
byte PH_ars[]={0x01,0x03,0x00,0x06,0x00,0x01,0x64,0x0B};
//土壤温度
byte Temp_ars[]={0x01,0x03,0x00,0x13,0x00,0x01,0x75,0xCF};
//土壤湿度
byte humid_ars[]={0x01,0x03,0x00,0x12,0x00,0x01,0x24,0x0F};
//土壤氮
byte N_ars[]={0x01,0x03,0x00,0x1E,0x00,0x01,0xE4,0x0C};
//土壤磷
byte P_ars[]={0x01,0x03,0x00,0x1F,0x00,0x01,0xB5,0xCC};
//土壤钾
byte K_ars[]={0x01,0x03,0x00,0x20,0x00,0x01,0x85,0xC0};


//主机问询命令
byte send_byte[6][8]={0x01,0x03,0x00,0x06,0x00,0x01,0x64,0x0B,//PH:NO.0
                    0x01,0x03,0x00,0x13,0x00,0x01,0x75,0xCF,//湿度:NO.1    
                    0x01,0x03,0x00,0x12,0x00,0x01,0x24,0x0F,//温度:NO.2
                    0x01,0x03,0x00,0x1E,0x00,0x01,0xE4,0x0C,//氮:NO.3
                    0x01,0x03,0x00,0x1F,0x00,0x01,0xB5,0xCC,//磷:NO.4
                    0x01,0x03,0x00,0x20,0x00,0x01,0x85,0xC0//钾:NO.5
};


/********变量********/

//传感器返回数据临时存储
byte temp[7];
//询问命令号:0-6
int asr=0;



/*************程序初始化*************/

void setup() 
{

  /*端口初始化*/
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial.printf("SYSTEM READY\n");
  delay(500);

}

void loop() 
{

  int i=0;

  /****端口读写****/
  for(i=0;i<8;i++)
  {
    Serial2.write(send_byte[asr][i]);
  }
 

  /****主机发送数据****/
  Serial.println("主机已发送数据:\n");
  Serial.printf("询问码NO:%d",asr);
  Serial.println();
  
  delay(200);
  
 /****从机应答数据****/
  if(Serial2.available())
  {
      
    Serial2.readBytes(temp,sizeof(temp));
    Serial.println("传感器应答数据:\n");
    parseModbusData(temp, sizeof(temp));
    if (checkCRC(temp, sizeof(temp))) 
    {
      Serial.println("\n传感器数据 CRC 校验成功\n");
      Serial.println();

      /*询问命令号更新*/
      asr++;
      if(asr==6)
        asr=0;
    }
    else 
    {
      Serial.println("\n传感器数据 CRC 校验失败\n");
      Serial.println();
    }

  }
  

  delay(2000);
}
  


/* CRC计算 */
uint16_t CRC16(const uint8_t *data, uint16_t length) 
{
  uint16_t crc = 0xFFFF;  // 初始值

  for (uint16_t i = 0; i < length; i++) {
      crc ^= data[i];  // 先与当前字节异或

      for (uint8_t j = 0; j < 8; j++) { // 处理 8 位
          if (crc & 0x0001) {
              crc = (crc >> 1) ^ 0xA001; // 多项式 0xA001
          } else {
              crc >>= 1;
          }
      }
  }
  return crc;  // 返回 CRC 结果
}


/* 校验CRC */
bool checkCRC(const uint8_t *data, uint16_t len) 
{
  if (len < 3) return false;
  uint16_t computedCRC = CRC16(data, len - 2);
  uint16_t receivedCRC = data[len - 2] | (data[len - 1] << 8);
  return computedCRC == receivedCRC;
}


/* 解析并打印数据 */
void parseModbusData(const uint8_t *data, uint16_t len) 
{
  Serial.println("解析数据:\n");

  // 打印数据帧
  Serial.print("十六进制:\n");
  for (uint16_t i = 0; i < len; i++) 
  {
    Serial.printf("%02X \n", data[i]);
  }
  Serial.println();

  // 解析寄存器值（假设数据位为 2 字节整数）
  if (data[1] == 0x03)
  {  // 读取寄存器
      uint16_t regValue = (data[3] << 8) | data[4];  // 高位在前，低位在后
      switch(asr)
      {
        case 0:
          Serial.printf("PH: %.1f \n",(double)regValue/10.0);
          break;
        
        case 1:
          Serial.printf("温度: %.1f°C \n",(double)regValue/10.0);
          break;
     
        case 2:
          Serial.printf("湿度: %.1f% \n",(double)regValue/10.0);
          break;
          
        case 3:
          Serial.printf("N: %.1fmg/kg \n",(double)regValue/10.0);
          break;

        case 4:
          Serial.printf("P: %.1fmg/kg \n",(double)regValue/10.0);
          break;

        case 5:
          Serial.printf("K: %.1fmg/kg \n",(double)regValue/10.0);
          break;

        default:
          Serial.printf("M ERROR!!!\n");
          break;
            
      }
  }


}