#define Flood 100

int ENB[] = {25, 27, 29, 31, 33, 35};
int RST = 37;
int SLP = 39;
int DIR = 41;
int STP[] = {43, 45, 47, 49, 51, 53};
int RELAY1 = 23;
int RELAY2 = 2;
int RELAY3 = 3;
int ENBActual;
int STPActual;

int Motor = 0;
int MotorSig = 0;
int MotorAnt = 0;
int Direccion[2];
int DirActual = 0;
int Ida = 0;
int Regreso = 0;
int DelayEntreIyR = 0;
int RepeticionActual = 0;
int CantidadRepeticiones = 0;
int DelayEntreRepeticiones = 0;
int DelayEntreAcciones = 0;

long PosicionActual[6];
long PosicionSiguiente[6];
long LimiteIzquierdo[6];
long LimiteDerecho[6];

int LDRActual = 0;
int PinesLDR[] = {0, 1, 2, 3, 4};
int ValorLDR = 0;
int EstadoLDR[] = {0, 0, 0, 0, 0};
long EstadoGlobal = 0;
int Filtros[] = {8, 10000, 1000, 100, 10, 10001, 11000, 1100, 110, 10011, 11001, 11100, 1110, 111, 11, 1};
int FiltroActual = 0;
int FiltroAColocar = 0;

int i = 0;
int MensajeErroneo = 0;
long PasoActual = 0;
volatile bool MotorBloqueado[] = {false, false, false, false, false, false};

typedef struct Message
{
  int ID;
  int Obj;
  int Acc;
  long Dist;
  int Dir;
  int Reg;
  int TiempoReg;
  int Rep;
  int TiempoRep;
  int DelayAcc;
} Message;

Message Mensaje[Flood];
int NumeroMensaje = 0;
char ArrayDist[8];
char ArrayTiempoReg[4];
char ArrayRep[3];
char ArrayTiempoRep[4];
char ArrayDelayAcc[4];


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// DECLARO LAS FUNCIONES QUE VOY A USAR /////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DarPaso(int i)
{
  STPActual = STP[i];
  digitalWrite(STPActual, HIGH);
  delayMicroseconds(15);
  digitalWrite(STPActual, LOW);
  delayMicroseconds(1200);
}


void HabilitarDriver(int i)
{
  ENBActual = ENB[i];
  MotorSig = ENBActual + 2;
  MotorAnt = ENBActual - 2;

  delay(1);
  digitalWrite(ENBActual, LOW);

  if (i == 0)
  {
    digitalWrite(MotorSig, LOW);
    MotorBloqueado[MotorSig] = false;
  }

  else if (i == 1)
  {
    digitalWrite(MotorSig, LOW);
    digitalWrite(MotorAnt, LOW);

    MotorBloqueado[MotorSig] = false;
    MotorBloqueado[MotorAnt] = false;
  }

  else if (i == 2)
  {
    digitalWrite(MotorAnt, LOW);
    MotorBloqueado[MotorAnt] = false;
  }

  digitalWrite(RST, HIGH);
  digitalWrite(SLP, HIGH);

  MotorBloqueado[i] = false;
}


void BloquearDriver(int i)
{
  ENBActual = ENB[i];
  MotorSig = ENBActual + 2;
  MotorAnt = ENBActual - 2;

  delay(1);
  digitalWrite(ENBActual, HIGH);

  if (i == 0)
  {
    digitalWrite(MotorSig, HIGH);
    MotorBloqueado[MotorSig] = true;

  }

  else if (i == 1)
  {
    digitalWrite(MotorSig, HIGH);
    digitalWrite(MotorAnt, HIGH);

    MotorBloqueado[MotorSig] = true;
    MotorBloqueado[MotorAnt] = true;
  }

  else if (i == 2)
  {
    digitalWrite(MotorAnt, HIGH);
    MotorBloqueado[MotorAnt] = true;
  }

  digitalWrite(RST, HIGH);
  digitalWrite(SLP, LOW);

  MotorBloqueado[i] = true;
}


int VerificarFiltro()
{
  for (i = 0; i < 5; i++)
  {
    LDRActual = PinesLDR[i];
    ValorLDR = analogRead(LDRActual);
    //Serial.print(ValorLDR);
    //Serial.print('-');

    if (ValorLDR < 850)
    {
      EstadoLDR[i] = 0;
    }

    else
    {
      EstadoLDR[i] = 1;
    }
  }

  EstadoGlobal = EstadoLDR[4] + 10 * EstadoLDR[3] + 100 * EstadoLDR[2] + 1000 * EstadoLDR[1] + 10000 * EstadoLDR[0];
  //Serial.print('\n');
  //Serial.print(EstadoGlobal);
  //Serial.print('\n');
  //Serial.print('\n');

  for (i = 0; i < 17; i++)
  {
    if (EstadoGlobal == Filtros[i])
    {
      FiltroActual = i;
    }
  }

  return FiltroActual;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void setup()
{
  for (i = 0; i < 6; i++)
  {
    PosicionActual[i] = 5000000;
    PosicionSiguiente[i] = 0;
    LimiteIzquierdo[i] = 99999999;
    LimiteDerecho[i] = 0;
  }

  for (i = 12; i < 28; i++)
  {
    int impares = (2 * i) - 1;
    pinMode(impares, OUTPUT);
  }

  for (i = 0; i < 6; i++)
  {
    MotorBloqueado[i] = true;
    ENBActual = ENB[i];
    digitalWrite(ENBActual, HIGH);
  }
  digitalWrite(RST, HIGH);
  digitalWrite(SLP, LOW);

  Serial.begin(115200, SERIAL_8N1);
  while (!Serial);
}



void loop()
{
  char serialData;

  while (Serial.available() >= 28 && NumeroMensaje < Flood)
  {
    ///////////////////////////////////////////////////////////////
    ////////////////// LEO EL IDENTIFICADOR ///////////////////////
    ///////////////////////////////////////////////////////////////
    serialData = Serial.read();

    if (serialData == (char)0xF0)
      return;

    if (!isdigit(serialData))
    {
      MensajeErroneo = 1;
    }

    switch (serialData)
    {
      case '0': Mensaje[NumeroMensaje].ID = 0;
        break;

      case '1': Mensaje[NumeroMensaje].ID = 1;
        break;

      case '2': Mensaje[NumeroMensaje].ID = 2;
        break;

      case '3': Mensaje[NumeroMensaje].ID = 3;
        break;

      case '4': Mensaje[NumeroMensaje].ID = 4;
        break;

      case '5': Mensaje[NumeroMensaje].ID = 5;
        break;

      case '8': Mensaje[NumeroMensaje].ID = 8;
        break;
    }
    ///////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////
    ////////////// LEO EL OBJETO (MOTOR o RELAY) //////////////////
    ///////////////////////////////////////////////////////////////
    serialData = Serial.read();

    if (!isdigit(serialData))
    {
      MensajeErroneo = 1;
    }

    switch (serialData)
    {
      case '0': Mensaje[NumeroMensaje].Obj = 0;
        break;

      case '1': Mensaje[NumeroMensaje].Obj = 1;
        break;

      case '2': Mensaje[NumeroMensaje].Obj = 2;
        break;

      case '3': Mensaje[NumeroMensaje].Obj = 3;
        break;

      case '4': Mensaje[NumeroMensaje].Obj = 4;
        break;

      case '5': Mensaje[NumeroMensaje].Obj = 5;
        break;

      case '8': Mensaje[NumeroMensaje].Obj = 8;
        break;
    }
    ///////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////
    // LEO LA ACCION (QUE TIPO DE GIRO, QUE LIMITE CONFIG, ETC.) //
    ///////////////////////////////////////////////////////////////
    serialData = Serial.read();

    if (!isdigit(serialData))
    {
      MensajeErroneo = 1;
    }

    switch (serialData)
    {
      case '0': Mensaje[NumeroMensaje].Acc = 0;
        break;

      case '1': Mensaje[NumeroMensaje].Acc = 1;
        break;

      case '2': Mensaje[NumeroMensaje].Acc = 2;
        break;

      case '8': Mensaje[NumeroMensaje].Acc = 8;
        break;
    }
    ///////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////
    ///////////////// LEO CANTIDAD DE PASOS ///////////////////////
    ///////////////////////////////////////////////////////////////
    for (i = 0; i < 8; i++)
    {
      ArrayDist[i] = Serial.read();

      if (!isdigit(ArrayDist[i]))
      {
        MensajeErroneo = 1;
      }
    }

    ArrayDist[i] = 0;
    Mensaje[NumeroMensaje].Dist = atol(ArrayDist);
    ///////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////
    /////////////////// LEO LA DIRECCION //////////////////////////
    ///////////////////////////////////////////////////////////////
    serialData = Serial.read();

    if (!isdigit(serialData))
    {
      MensajeErroneo = 1;
    }

    switch (serialData)
    {
      case '0': Mensaje[NumeroMensaje].Dir = 0;
        break;

      case '1': Mensaje[NumeroMensaje].Dir = 1;
        break;

      case '8': Mensaje[NumeroMensaje].Dir = 8;
        break;
    }
    ///////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////
    /////////////// LEO SI TIENE QUE REGRESAR /////////////////////
    ///////////////////////////////////////////////////////////////
    serialData = Serial.read();

    if (!isdigit(serialData))
    {
      MensajeErroneo = 1;
    }

    switch (serialData)
    {
      case '0': Mensaje[NumeroMensaje].Reg = 0;
        break;

      case '1': Mensaje[NumeroMensaje].Reg = 1;
        break;

      case '8': Mensaje[NumeroMensaje].Reg = 8;
        break;
    }
    ///////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////
    ////////////////// LEO TIEMPO ENTRE IyR ///////////////////////
    ///////////////////////////////////////////////////////////////
    for (i = 0; i < 4; i++)
    {
      ArrayTiempoReg[i] = Serial.read();

      if (!isdigit(ArrayTiempoReg[i]))
      {
        MensajeErroneo = 1;
      }
    }

    ArrayTiempoReg[i] = 0;
    Mensaje[NumeroMensaje].TiempoReg = atoi(ArrayTiempoReg);
    ///////////////////////////////////////////////////////////////



    ///////////////////////////////////////////////////////////////
    //////////////// LEO CUANTAS VECES REPITE /////////////////////
    ///////////////////////////////////////////////////////////////
    for (i = 0; i < 3; i++)
    {
      ArrayTiempoReg[i] = Serial.read();

      if (!isdigit(ArrayTiempoReg[i]))
      {
        MensajeErroneo = 1;
      }
    }

    ArrayTiempoReg[i] = 0;
    Mensaje[NumeroMensaje].Rep = atoi(ArrayTiempoReg);
    ///////////////////////////////////////////////////////////////



    ///////////////////////////////////////////////////////////////
    ////////////////// LEO TIEMPO ENTRE REP ///////////////////////
    ///////////////////////////////////////////////////////////////
    for (i = 0; i < 4; i++)
    {
      ArrayTiempoReg[i] = Serial.read();

      if (!isdigit(ArrayTiempoReg[i]))
      {
        MensajeErroneo = 1;
      }
    }

    ArrayTiempoReg[i] = 0;
    Mensaje[NumeroMensaje].TiempoRep = atoi(ArrayTiempoReg);
    ///////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////
    ////////////////// LEO TIEMPO ENTRE ACC ///////////////////////
    ///////////////////////////////////////////////////////////////
    for (i = 0; i < 4; i++)
    {
      ArrayDelayAcc[i] = Serial.read();

      if (!isdigit(ArrayDelayAcc[i]))
      {
        MensajeErroneo = 1;
      }
    }

    ArrayDelayAcc[i] = 0;
    Mensaje[NumeroMensaje].DelayAcc = atoi(ArrayDelayAcc);
    ///////////////////////////////////////////////////////////////

    NumeroMensaje++;
  }



  if (NumeroMensaje > 0 && MensajeErroneo == 0)
  {
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////// LEO EL VALOR DEL IDENTIFICADOR. SI ES 0, ENTONCES CORRESPONDE A MOVER UN MOTOR ////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (Mensaje[0].ID == 0)
    {
      if (PasoActual == 0)
      {
        Motor = Mensaje[0].Obj;

        //////////////////////////////////////////////////////////////////////
        ///////////////// LE ASIGNO LA DIRECCION INICIAL /////////////////////
        //////////////////////////////////////////////////////////////////////
        if (Mensaje[0].Dir == 0)
        {
          Direccion[0] = 0;
          Direccion[1] = 1;
        }

        else if (Mensaje[0].Dir == 1)
        {
          Direccion[0] = 1;
          Direccion[1] = 0;
        }
        //////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////
        //////////// ¿REGRESA AL TERMINAR?¿CUANTO TIEMPO DESPUES? ////////////
        //////////////////////////////////////////////////////////////////////
        Regreso = Mensaje[0].Reg;
        DelayEntreIyR = (1000 * Mensaje[0].TiempoReg);
        //////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////
        //////////// ¿REGRESA AL TERMINAR?¿CUANTO TIEMPO DESPUES? ////////////
        //////////////////////////////////////////////////////////////////////
        CantidadRepeticiones = Mensaje[0].Rep;
        DelayEntreRepeticiones = (1000 * Mensaje[0].TiempoRep);
        //////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////
        ////////////// ¿TIEMPO ENTRE UNA ACCION Y LA SIGUIENTE? //////////////
        //////////////////////////////////////////////////////////////////////
        DelayEntreAcciones = (1000 * Mensaje[0].DelayAcc);
        //////////////////////////////////////////////////////////////////////
      }


      if (Mensaje[0].Acc == 0)
      {
        if (CantidadRepeticiones == 999)
        {
          RepeticionActual = 0;
        }

        if (RepeticionActual < CantidadRepeticiones + 1)
        {
          if (Ida < Regreso + 1)
          {
            DirActual = Direccion[Ida];

            if (DirActual == 0 && PasoActual == 0)
            {
              HabilitarDriver(Motor);
              digitalWrite(DIR, LOW);
              delay(25);
            }

            if (DirActual == 1 && PasoActual == 0)
            {
              HabilitarDriver(Motor);
              digitalWrite(DIR, HIGH);
              delay(25);
            }


            if (PasoActual < Mensaje[0].Dist)
            {
              PosicionSiguiente[Motor] = PosicionActual[Motor] - pow(-1, DirActual);
Serial.print(LimiteDerecho[4]);
// ESTE ES EL SERIAL PRINT QUE LE AGREGUE Y ME SOLUCIONA TODO.
              if (LimiteIzquierdo[Motor] > PosicionSiguiente[Motor] && PosicionSiguiente[Motor] > LimiteDerecho[Motor])
              {
                DarPaso(Motor);

                PosicionActual[Motor] = PosicionActual[Motor] - pow(-1, DirActual);
                PasoActual++;
              }

              else
              {
                if (!MotorBloqueado[Motor])
                {
                  BloquearDriver(Motor);
                  PasoActual++;
                }

                else
                {
                  PasoActual++;
                }
              }
            }

            else
            {
              BloquearDriver(Motor);
              PasoActual = 0;
              Ida++;

              if (Ida == 1)
              {
                delay(DelayEntreIyR);
              }
            }
          }

          else
          {
            Ida = 0;
            RepeticionActual++;

            if (RepeticionActual <= CantidadRepeticiones)
            {
              delay(DelayEntreRepeticiones);
            }
          }
        }

        else
        {
          RepeticionActual = 0;
          delay(DelayEntreAcciones);

          NumeroMensaje--;
          memmove(Mensaje, Mensaje + 1, NumeroMensaje * sizeof(Message));
        }
      }

      else if (Mensaje[0].Acc == 1)
      {
        PasoActual = 0;

        if (PasoActual == 0)
        {
          HabilitarDriver(Motor);
          digitalWrite(DIR, LOW);
          delay(25);
        }

        FiltroActual = VerificarFiltro();
        FiltroAColocar = Mensaje[0].Dist;
        while (FiltroActual != FiltroAColocar)
        {
          DarPaso(Motor);
          PasoActual++;
          FiltroActual = VerificarFiltro();

          if (PasoActual > 400)
          //if (PasoActual > 1)
          {
            FiltroActual = FiltroAColocar;
          }
        }

        BloquearDriver(Motor);
        NumeroMensaje--;
        memmove(Mensaje, Mensaje + 1, NumeroMensaje * sizeof(Message));
      }

      else
      {
        NumeroMensaje--;
        memmove(Mensaje, Mensaje + 1, NumeroMensaje * sizeof(Message));
      }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////// LEO EL VALOR DEL IDENTIFICADOR. SI ES 1, ENTONCES CORRESPONDE A SETEAR LIMITES ////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else if (Mensaje[0].ID == 1)
    {
      Motor = Mensaje[0].Obj;

      if (Mensaje[0].Acc == 0)
      {
        LimiteDerecho[Motor] = PosicionActual[Motor] - 1;
      }

      else if (Mensaje[0].Acc == 1)
      {
        LimiteIzquierdo[Motor] = PosicionActual[Motor] + 1;
      }

      else if (Mensaje[0].Acc == 2)
      {
        LimiteIzquierdo[Motor] = 99999999;
        LimiteDerecho[Motor] = 0;
        PosicionActual[Motor] = 5000000;
      }

      NumeroMensaje--;
      memmove(Mensaje, Mensaje + 1, NumeroMensaje * sizeof(Message));
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////// LEO EL VALOR DEL IDENTIFICADOR. SI ES 2, ENTONCES CORRESPONDE A COMANDAR RELAY ////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else if (Mensaje[0].ID == 2)
    {
      if (Mensaje[0].Obj == 0)
      {
        if (Mensaje[0].Acc == 0)
        {
          digitalWrite(RELAY1, LOW);
        }

        else if (Mensaje[0].Acc == 1)
        {
          digitalWrite(RELAY1, HIGH);
        }
      }


      else if (Mensaje[0].Obj == 1)
      {
        if (Mensaje[0].Acc == 0)
        {
          digitalWrite(RELAY2, LOW);
        }

        else if (Mensaje[0].Acc == 1)
        {
          digitalWrite(RELAY2, HIGH);
        }
      }


      else if (Mensaje[0].Obj == 2)
      {
        if (Mensaje[0].Acc == 0)
        {
          digitalWrite(RELAY3, LOW);
        }

        else if (Mensaje[0].Acc == 1)
        {
          digitalWrite(RELAY3, HIGH);
        }
      }

      NumeroMensaje--;
      memmove(Mensaje, Mensaje + 1, NumeroMensaje * sizeof(Message));
    }

    else
    {
      NumeroMensaje--;
      memmove(Mensaje, Mensaje + 1, NumeroMensaje * sizeof(Message));
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  }

  else if (MensajeErroneo == 1)
  {
    for (i = 0; i < 6; i++)
    {
      BloquearDriver(i);
      digitalWrite(STP, LOW);
    }

    if (Serial.available() > 0)
    {
      serialData = Serial.read();
      NumeroMensaje = 0;
    }

    else
    {
      PasoActual = 0;
      Mensaje[0].Dist = 0;
      Ida = 0;
      Regreso = 0;
      RepeticionActual = 0;
      CantidadRepeticiones = 0;
      DelayEntreIyR = 0;
      DelayEntreRepeticiones = 0;
      DelayEntreAcciones = 0;

      NumeroMensaje = 0;

      MensajeErroneo = 0;
    }
  }
}
