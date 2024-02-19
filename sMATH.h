int getGCD(int a, int b)
{
int tmp;
double r;
  if (b > a) {
    tmp = b;
    b = a;
    a =  tmp;
  }

  while (b != 0) {
    r = a % b;
    a = b;
    b = (int)r;
  }

  return a;
}

int SerialPrintDouble(double dub)
{
uint32_t in,fr;
double mil;

  mil=dub/1000;
  in=(uint32_t)mil;
  fr=(uint32_t)round((mil-(double)in)*1000);

  if(in>0)
    Serial.print(in);

  if(fr<100)
    Serial.print("0");
  if(fr<10)
    Serial.print("0");
  if(fr<1)
    Serial.print("0");
  else
    Serial.print(fr);
  return 0;
}
