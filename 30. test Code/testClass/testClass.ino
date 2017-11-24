

//outerB.h

//嵌套类 innerA
class innerA
{
  public:
    innerA();
    ~innerA();
    void _show();
};

//外围类outerB
class outerB
{
  public:
    outerB();
    ~outerB();
    void show();
  innerA classa;

  private:
    innerA*  m_p_innerA;
};



//outerB.cpp
outerB::outerB(): m_p_innerA(new innerA)
{
  printf("%s \r\n", __FUNCTION__);
}

outerB::~outerB()
{
  Serial.printf("%s \r\n", __FUNCTION__);
  if (m_p_innerA != NULL)
  {
    delete m_p_innerA;
  }
}


void outerB::show()
{
  m_p_innerA->_show();
  printf("%s \r\n", __FUNCTION__);
}
outerB::innerA::innerA()
{
  Serial.printf("%s \r\n", __FUNCTION__);
}
outerB::innerA::~innerA()
{
  Serial.printf("%s \r\n", __FUNCTION__);
}


void outerB::innerA::_show()
{
  Serial.printf("%s \r\n", __FUNCTION__);
}

int main()
{
  outerB x;
  x.show();
  return 0;
}

//输出
/*
  outerB::innerA::innerA
  outerB::outerB
  outerB::innerA::_show
  outerB::show
  outerB::~outerB
  outerB::innerA::~innerA
*/

void setup()
{
  Serial.begin(115200);
  Serial.printf("\n\n");
  main();
}

void loop()
{

}

