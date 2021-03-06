#include "ModbusServer.hpp"

#define DIGITAL_INPUT_CAP 14
#define ANALOG_OUTPUT_FACTOR 4
#define ONE 0x00000001
#define ZERO 0x00000000
#define M128 0x80
#define DIGITAL_SIZE 20
#define ANALOG_SIZE 10

using namespace std;

ModbusServer::ModbusServer(byte id)
{
  this->id = id;
  this->start_milis = clock();
  time(&this->start_seconds);
  SetData();
  cout << "===== Datos Iniciales =====" << endl;
  PrintVectors();
  cout << endl;
}

ModbusServer::~ModbusServer() { }

vector<byte> ModbusServer::peticion(vector<byte> recibido)
{
  vector<byte> output (0);

  UpdateData(recibido.size());


  if (recibido[0] != this->id)
  {
    PrintVectors();
    return output;
  }

  if (!CheckCRC(recibido))
  {
    PrintVectors();
    return output;
  }

  switch (recibido[1])
  {
    case 0x01:
      output = ReadDigitalOutput_01(recibido);
      this->analog_input[14] += output.size();
      PrintVectors();
      return output;
      break;
    case 0x02:
      output = ReadDigitalInput_02(recibido);
      this->analog_input[14] += output.size();
      PrintVectors();
      return output;
      break;
    case 0x03:
      output = ReadAnalogOutput_03(recibido);
      this->analog_input[14] += output.size();
      PrintVectors();
      return output;
      break;
    case 0x04:
      output = WriteAnalogInput_04(recibido);
      this->analog_input[14] += output.size();
      PrintVectors();
      return output;
      break;
    case 0x05:
      output = WriteDigitalOutput_05(recibido);
      this->analog_input[14] += output.size();
      PrintVectors();
      return output;
      break;
    case 0x06:
      output = WriteAnalogOutput_06(recibido);
      this->analog_input[14] += output.size();
      PrintVectors();
      return output;
      break;
    case 0x0F:
      output = WriteDigitalOutputMultiple_15(recibido);
      this->analog_input[14] += output.size();
      PrintVectors();
      return output;
      break;
    case 0x10:
      output = WriteAnalogOutputMultiple_16(recibido);
      this->analog_input[14] += output.size();
      PrintVectors();
      return output;
      break;
    default:
      output = ErrorIllegalFunction_01(recibido);
      this->analog_input[14] += output.size();
      PrintVectors();
      return output;
      break;
  }

  return output;
}

vector<byte> ModbusServer::ReadDigitalOutput_01(vector<byte> input)
{
  vector<byte> output;
  int i, j;
  byte coils_value = 0x0;

  output.push_back(input[0]);
  output.push_back(input[1]);
  int coils_to_read = BytesToInt(input[4], input[5]);
  int bytes = (byte) (ceil( (float) coils_to_read/8 ));
  output.push_back(bytes);
  int coils_address = BytesToInt(input[2], input[3]);

  if (coils_address < ZERO or coils_address > DIGITAL_SIZE )
    return ErrorIllegalDataAddress_02(input);
  if (coils_address + coils_to_read < ZERO or coils_address + coils_to_read > DIGITAL_SIZE )
    return ErrorIllegalDataAddress_02(input);

  for (i = 0; i < bytes; i++)
  {
    int j_inicio, j_fin;

    j_inicio = coils_address + 8 * i;
    if (coils_to_read - 8 > 0)
    {
      j_fin = j_inicio + 8;
      coils_to_read -= 8;
    }
    else
      j_fin = j_inicio + coils_to_read;

    for (j = j_fin; j >  j_inicio; j--)
    {
      if (this->digital_output[j-1])
      {
        coils_value <<= 1;
        coils_value ^= ONE;
      }
      else
      {
        coils_value <<= 1;
        coils_value ^= ZERO;
      }
    }
    output.push_back(coils_value);
    coils_value = ZERO;
  }

  AddVector(&output, CRC16(output));

  return output;
}

vector<byte> ModbusServer::ReadDigitalInput_02(vector<byte> input)
{
vector<byte> output;
  int i, j;
  byte coils_value = 0x0;

  output.push_back(input[0]);
  output.push_back(input[1]);
  int coils_to_read = BytesToInt(input[4], input[5]);
  int bytes = (byte) (ceil( (float) coils_to_read/8 ));
  output.push_back(bytes);
  int coils_address = BytesToInt(input[2], input[3]);

  if (coils_address < ZERO or coils_address > DIGITAL_SIZE )
    return ErrorIllegalDataAddress_02(input);
  if (coils_address + coils_to_read < ZERO or coils_address + coils_to_read > DIGITAL_SIZE )
    return ErrorIllegalDataAddress_02(input);

  for (i = 0; i < bytes; i++)
  {
    int j_inicio, j_fin;

    j_inicio = coils_address + 8 * i;
    if (coils_to_read - 8 > 0)
    {
      j_fin = j_inicio + 8;
      coils_to_read -= 8;
    }
    else
      j_fin = j_inicio + coils_to_read;

    for (j = j_fin; j >  j_inicio; j--)
    {
      if (this->digital_output[j-1])
      {
        coils_value <<= 1;
        coils_value ^= ONE;
      }
      else
      {
        coils_value <<= 1;
        coils_value ^= ZERO;
      }
    }
    output.push_back(coils_value);
    coils_value = ZERO;
  }

  AddVector(&output, CRC16(output));

  return output;
}

vector<byte> ModbusServer::ReadAnalogOutput_03(vector<byte> input)
{
  vector<byte> output;
  int i = 0;

  output.push_back(input[0]);
  output.push_back(input[1]);
  int coils_to_read = BytesToInt(input[4], input[5]);
  output.push_back((byte)(2 * coils_to_read));
  int coils_address = BytesToInt(input[2], input[3]);

  if (coils_address < ZERO or coils_address > ANALOG_SIZE )
    return ErrorIllegalDataAddress_02(input);
  if (coils_address + coils_to_read < ZERO or coils_address + coils_to_read > ANALOG_SIZE )
    return ErrorIllegalDataAddress_02(input);

  for (i = 0; i < coils_to_read; i++)
    AddVector(&output, IntToByte(this->analog_output[coils_address + i]));

  AddVector(&output, CRC16(output));

  return output;
}

vector<byte> ModbusServer::WriteAnalogInput_04(vector<byte> input)
{
  vector<byte> output;
  int i;

  if (input.size() != 8)
    return ErrorIllegalDataValue_03(input);

  output.push_back(input[0]);
  output.push_back(input[1]);
  int coils_address = BytesToInt(input[2], input[3]);
  int coils_to_read = BytesToInt(input[4], input[5]);
  output.push_back((byte)(2 * coils_to_read));

  for (i = 0; i < coils_to_read; i++)
    AddVector(&output, IntToByte(this->analog_input[coils_address + i]));

  AddVector(&output, CRC16(output));

  return output;
}

vector<byte> ModbusServer::WriteDigitalOutput_05(vector<byte> input)
{
  vector<byte> output;
  int operation;

  output.push_back(input[0]);
  output.push_back(input[1]);
  output.push_back(input[2]);
  output.push_back(input[3]);

  int coils_address = BytesToInt(input[2], input[3]);

  if (coils_address < ZERO or coils_address > DIGITAL_SIZE )
    return ErrorIllegalDataAddress_02(input);

  operation = BytesToInt(input[4], input[5]);

  if (operation != 0xFF00 && operation != 0x0000 )
    return ErrorIllegalDataValue_03(input);

  if (operation == 0xFF00)
  {
    this->digital_output[coils_address] = true;
    output.push_back(input[4]);
    output.push_back(input[5]);
  }
  else
  {
    this->digital_output[coils_address] = false;
    output.push_back(input[4]);
    output.push_back(input[5]);
  }

  AddVector(&output, CRC16(output));

  return output;
}

vector<byte> ModbusServer::WriteAnalogOutput_06(vector<byte> input)
{
  vector<byte> output;

  output.push_back(input[0]);
  output.push_back(input[1]);
  output.push_back(input[2]);
  output.push_back(input[3]);
  output.push_back(input[4]);
  output.push_back(input[5]);

  int coils_address = BytesToInt(input[2], input[3]);
  int coils_value = BytesToInt(input[4], input[5]);

  if (coils_address < ZERO or coils_address > ANALOG_SIZE )
    return ErrorIllegalDataAddress_02(input);

  this->analog_output[coils_address] = coils_value;

  AddVector(&output, CRC16(output));

  return output;
}

vector<byte> ModbusServer::WriteDigitalOutputMultiple_15(vector<byte> input)
{
  vector<byte> output;
  int i, j;

  output.push_back(input[0]);
  output.push_back(input[1]);
  output.push_back(input[2]);
  output.push_back(input[3]);
  output.push_back(input[4]);
  output.push_back(input[5]);

  int coils_address = BytesToInt(input[2], input[3]);
  int coils_to_read = BytesToInt(input[4], input[5]);
  int bytes = ByteToInt(input[6]);

  if (coils_address < ZERO or coils_address > DIGITAL_SIZE )
    return ErrorIllegalDataAddress_02(input);
  if (coils_address + coils_to_read < ZERO or coils_address + coils_to_read > DIGITAL_SIZE )
    return ErrorIllegalDataAddress_02(input);

  for (i = 0; i < bytes; i++)
  {
    int j_inicio, j_fin;

    j_inicio = coils_address + 8 * i;
    if (coils_to_read - 8 > 0)
    {
      j_fin = j_inicio + 8;
      coils_to_read -= 8;
    }
    else
      j_fin = j_inicio + coils_to_read;

    int byte = ByteToInt(input[i+7]);
    for (j = j_inicio; j < j_fin; j++)
    {
      bool coil = byte & ONE;
      byte >>= 1;

      this->digital_output[j] = coil;
    }
  }

  AddVector(&output, CRC16(output));

  return output;
}

vector<byte> ModbusServer::WriteAnalogOutputMultiple_16(vector<byte> input)
{
  vector<byte> output;
  int i;

  output.push_back(input[0]);
  output.push_back(input[1]);
  output.push_back(input[2]);
  output.push_back(input[3]);
  output.push_back(input[4]);
  output.push_back(input[5]);

  int coils_address = BytesToInt(input[2], input[3]);
  int coils_bytes = BytesToInt(input[4], input[5]);
  int coils_to_read = ByteToInt(input[6]);

  if (coils_address < ZERO or coils_address > ANALOG_SIZE )
    return ErrorIllegalDataAddress_02(input);
  if (coils_address + coils_bytes < ZERO or coils_address + coils_bytes > ANALOG_SIZE )
    return ErrorIllegalDataAddress_02(input);
  if (input.size() != (unsigned)coils_to_read + 9)
    return ErrorIllegalDataValue_03(input);

  vector<int> intermediate;
  for (i = 0; i < coils_to_read; i += 2)
    intermediate.push_back(BytesToInt(input[7 + i], input[7 + i + 1]));

  for (i = 0; i < coils_to_read/2; i++)
    this->analog_output[coils_address+i] = intermediate[i];


  AddVector(&output, CRC16(output));

  return output;
}

void ModbusServer::PrintDigitalVector(vector<bool> input, string str)
{
  unsigned int i;

  cout << "Digital " << str << " -> [ ";
  for (i = 0; i < input.size(); i++)
    cout << input[i] << " ";
  cout << "]" << endl;
}

void ModbusServer::PrintAnalogVector(vector<int> input, string str)
{
  unsigned int i;

  cout << "Analog " << str << " -> [ ";
  for (i = 0; i < input.size(); i++)
    cout << input[i] << " ";
  cout << "]" << endl;
}

int ModbusServer::BytesToInt(byte byte1, byte byte2)
{
  return ( (int) byte2 ) + ( (int) byte1 << 8 );
}

int ModbusServer::ByteToInt(byte byte1)
{
  return ( (int) byte1);
}

vector<byte> ModbusServer::IntToByte(int input)
{
  vector<byte> output;

  output.push_back((char)(input >> 8));
  output.push_back((char)input);

  return output;
}

void ModbusServer::AddVector(vector<byte> *vector_1, vector<byte> vector_2)
{
  unsigned int i;
  for (i = 0; i < vector_2.size(); i++)
    vector_1->push_back(vector_2[i]);
}

/*! Calcula el código CRC-16 de los primeros 'len' bytes
 * \param mensaje vector que contiene los bytes de trabajo
 * \param len numero de bytes considerados
 * \return vector con los dos bytes del CRC en el orden "correcto"
*/
vector<byte>  ModbusServer::CRC16( vector<byte> mensaje, int len)
{
#define POLY 0xA001
  int i;
  unsigned int crc=0xFFFF;


  for(int ba=0; ba<len; ba++) {
    crc ^= mensaje[ba];
    for(i=0; i<8; i++) {
      if( crc & 0x0001 ) {
        crc = (crc>>1) ^ POLY;
      } else {
        crc >>= 1;
      }
    }
  }
  vector<byte> codr;
  codr.push_back(crc & 0xff);
  codr.push_back(crc >> 8);
  return codr;
}

/*! Calcula el código CRC-16 de TODOS los bytes del vector
 * \param mensaje vector de bytes de trabajo
 * \return vector con los dos bytes del CRC en el orden "correcto"
 */
vector<byte>  ModbusServer::CRC16( vector<byte> mensaje)
{
  return CRC16( mensaje, mensaje.size() );
}

void ModbusServer::PrintVectors(void)
{
  PrintDigitalVector(this->digital_output, "Output");
  PrintDigitalVector(this->digital_input, "Input");
  PrintAnalogVector(this->analog_output, "Output");
  PrintAnalogVector(this->analog_input, "Input");
}

void ModbusServer::SetData()
{
  unsigned int i;

  this->digital_output = vector<bool> (20, false);
  this->digital_input = vector<bool> (20, false);
  this->analog_output = vector<int> (10, 0);
  this->analog_input = vector<int> (20, 0);

  //Digital Output
  for (i = 0; i < this->digital_output.size(); i+=2)
    this->digital_output[i] = true;

  //Analog Output
  for (i = 0; i < this->analog_output.size(); i++)
    this->analog_output[i] = i * ANALOG_OUTPUT_FACTOR;

  //Analog Input
  for (i = 15; i < this->analog_input.size(); i++)
    if (i % 2 == 0)
      this->analog_input[i] = 1111;
    else
      this->analog_input[i] = 0;

  time_t timer = time(NULL);
  tm* y2k = localtime(&timer);

  //Los 6 [0-5] primeros: año, mes, dia, hora, minuto, segundo
  this->analog_input[0] = y2k->tm_year + 1900;
  this->analog_input[1] = y2k->tm_mon + 1;
  this->analog_input[2] = y2k->tm_mday;
  this->analog_input[3] = y2k->tm_hour;
  this->analog_input[4] = y2k->tm_min;
  this->analog_input[5] = y2k->tm_sec;
  //Los 4 [6-9] siguientes: UID, GID, PID, PPID
  this->analog_input[6] = getuid();
  this->analog_input[7] = getgid();
  this->analog_input[8] = getpid();
  this->analog_input[9] = getppid();
  //Los 2 [10-11] siguientes: segundos y milisegundos de computo del proceso
  this->analog_input[10] = 0;
  this->analog_input[11] = 0;
  //Los 3 [12-14] siguientes: contador peticiones recibidas, numero de bytes recibido, numero de bytes enviado
  this->analog_input[12] = 0;
  this->analog_input[13] = 0;
  this->analog_input[14] = 0;

  //Digital Input
  for (i = 0; i < DIGITAL_INPUT_CAP; i++)
    if(this->analog_input[i] % 2 == 0)
      this->digital_input[i] = true;
}

void ModbusServer::UpdateData(int bytes_recibidos)
{
  unsigned int i;

  //Analog Input
  for (i = 15; i < this->analog_input.size(); i++)
    if (i % 2 == 0)
      this->analog_input[i] = 1111;
    else
      this->analog_input[i] = 0;

  time_t timer = time(NULL);
  tm* y2k = localtime(&timer);

  //Los 6 [0-5] primeros: año, mes, dia, hora, minuto, segundo
  this->analog_input[0] = y2k->tm_year + 1900;
  this->analog_input[1] = y2k->tm_mon + 1;
  this->analog_input[2] = y2k->tm_mday;
  this->analog_input[3] = y2k->tm_hour;
  this->analog_input[4] = y2k->tm_min;
  this->analog_input[5] = y2k->tm_sec;

  //Los 4 [6-9] siguientes: UID, GID, PID, PPID
  this->analog_input[6] = getuid();
  this->analog_input[7] = getgid();
  this->analog_input[8] = getpid();
  this->analog_input[9] = getppid();

  //Los 2 [10-11] siguientes: segundos y milisegundos de computo del proceso
  time_t stop_seconds = time(&stop_seconds);
  this->analog_input[10] = (int) difftime(stop_seconds, this->start_seconds);

  clock_t stop_milis = clock();
  double milis = double (stop_milis - this->start_milis) / CLOCKS_PER_SEC;
  this->analog_input[11] = (int) (milis * 1000);

  //Los 3 [12-14] siguientes: contador peticiones recibidas, numero de bytes recibido, numero de bytes enviado
  this->analog_input[12] += 1;
  this->analog_input[13] += bytes_recibidos;

  //Digital Input
  for (i = 0; i < DIGITAL_INPUT_CAP; i++)
    if(this->analog_input[i] % 2 == 0)
      this->digital_input[i] = true;
}

bool ModbusServer::CheckCRC(vector<byte> input)
{
  unsigned int i;
  vector<byte> crc_nuevo;
  vector<byte> crc_viejo;

  for (i = 0; i < input.size() - 2; i++)
    crc_nuevo.push_back(input[i]);

  crc_nuevo = CRC16(crc_nuevo);
  crc_viejo.push_back(input[input.size() - 2]);
  crc_viejo.push_back(input[input.size() - 1]);

  for (i = 0; i < 2; i++)
    if (crc_nuevo[i] != crc_viejo[i])
      return false;

  return true;
}

vector<byte> ModbusServer::ErrorIllegalFunction_01(vector<byte> input)
{
  vector<byte> output;

  output.push_back(input[0]);
  output.push_back(input[1] ^ M128);
  output.push_back(0x01);

  AddVector(&output, CRC16(output));

  return output;
}

vector<byte> ModbusServer::ErrorIllegalDataAddress_02(vector<byte> input)
{
  vector<byte> output;

  output.push_back(input[0]);
  output.push_back(input[1] ^ M128);
  output.push_back(0x02);

  AddVector(&output, CRC16(output));

  return output;
}

vector<byte> ModbusServer::ErrorIllegalDataValue_03(vector<byte> input)
{
  vector<byte> output;

  output.push_back(input[0]);
  output.push_back(input[1] ^ M128);
  output.push_back(0x03);

  AddVector(&output, CRC16(output));

  return output;
}
