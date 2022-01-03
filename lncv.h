/*************************************************************************/
/*          EEPROM FUNCTIONS                                             */
/*************************************************************************/
void saveSettings()
{
  int i = 0;

  EEPROM.write(0, VERSION);
  
  for (i = 0; i < LNCV_COUNT; i++)
  {
    EEPROM.write(i * 2 + 2, highByte(lncv[i]));
    EEPROM.write(i * 2 + 3, lowByte(lncv[i]));
  }
}

void resetSettings()
{
  #ifdef DEBUG
  Serial.print("RESET MODULE CONFIGURATION");
  #endif

  lncv[0] = 0;
  lncv[1] = 1531;  // LN address for wecker #1
  lncv[2] = 1;    // start (sensor) address
  lncv[3] = 4;  // No of mp3 sounds
  
  saveSettings();
}

void loadSettings()
{
  int i = 0;

  //Check if there is no configuration stored or major version changed to reset
  if (EEPROM.read(0) != VERSION)
  {
    resetSettings();
    return;
  }

  for (i = 0; i < LNCV_COUNT; i++)
  {
    lncv[i] = word(EEPROM.read(i * 2 + 2), EEPROM.read(i * 2 + 3));    
  }
  if (lncv[0]==0) resetSettings();
}
/*************************************************************************/
/*          LNCV LOCONET PROGRAMMING FUNCTIONS                           */
/*************************************************************************/
void commitLNCVUpdate() {
  #ifdef DEBUG
    Serial.print("LNCV Commit Update, module Address is now: ");
    Serial.print(lncv[0]);
    Serial.print("\n");
  #endif
  saveSettings();
}

int8_t notifyLNCVread(uint16_t ArtNr, uint16_t lncvAddress, uint16_t, uint16_t & lncvValue)
{
  #ifdef DEBUG
    Serial.print("READ LNCV (");
    Serial.print(lncvAddress);
    Serial.print("): ");
  #endif

  // Step 1: Can this be addressed to me?
  // All ReadRequests contain the ARTNR. For starting programming, we do not accept the broadcast address.
  if (modeProgramming)
  {
    if (ArtNr == ARTNR)
    {
      if (lncvAddress < LNCV_COUNT)
      {
        lncvValue = lncv[lncvAddress];
        #ifdef DEBUG
          Serial.print(" LNCV Value: ");
          Serial.print(lncvValue);
          Serial.print("\n");
        #endif
         
        return LNCV_LACK_OK;
      }
      else
      {
        // Invalid LNCV address, request a NAXK
        #ifdef DEBUG
          Serial.print(" Invalid LNCV addres\n");
        #endif
        return LNCV_LACK_ERROR_UNSUPPORTED;
      }
    }
    else
    {
      #ifdef DEBUG
        Serial.print("ArtNr invalid.\n");
      #endif
      return -1;
    }
  }
  else
  {
    #ifdef DEBUG
      Serial.print("Ignoring Request.\n");
    #endif
    return -1;
  }
}

int8_t notifyLNCVprogrammingStart(uint16_t & ArtNr, uint16_t & ModuleAddress)
{
  // Enter programming mode. If we already are in programming mode,
  // we simply send a response and nothing else happens.
  #ifdef DEBUG
    Serial.print("LNCV PROGRAMMING START: ");
    Serial.print(ArtNr);Serial.print("-");Serial.println(ModuleAddress);
  #endif
  if (ArtNr == ARTNR)
  {
    if (ModuleAddress == lncv[0])
    {
      modeProgramming = true;
      return LNCV_LACK_OK;
    }
    else if (ModuleAddress == 0xFFFF)
    {
      ModuleAddress = lncv[0];
      return LNCV_LACK_OK;
    }
  }  
  return -1;
}

int8_t notifyLNCVwrite(uint16_t ArtNr, uint16_t lncvAddress, uint16_t lncvValue)
{

  int n = 0;

  if (!modeProgramming)
  {    
    return -1;
  }

  #ifdef DEBUG
    Serial.print("LNCV WRITE (LNCV ");
    Serial.print(lncvAddress); Serial.print(" = "); Serial.print(lncvValue);
    Serial.print("): ");
  #endif

  if (ArtNr == ARTNR)
  {
    //Valid LNCV number
    if (lncvAddress < LNCV_COUNT)
    {      
      lncv[lncvAddress] = lncvValue;
      return LNCV_LACK_OK;
    }
    else
    {      
      return LNCV_LACK_ERROR_UNSUPPORTED;
    }
  }
  else
  {    
    return -1;
  }
}

void notifyLNCVprogrammingStop(uint16_t ArtNr, uint16_t ModuleAddress)
{
  if (modeProgramming)
  {
    if (ArtNr == ARTNR && ModuleAddress == lncv[0])
    {
      #ifdef DEBUG
        Serial.print("LNCV PROGRAMMING STOP: ");
        Serial.print(ArtNr);Serial.print("-");Serial.println(ModuleAddress);
      #endif
      modeProgramming = false;
      commitLNCVUpdate();
    }
    else
    {
      if (ArtNr != ARTNR)
      {
        return;
      }
      if (ModuleAddress != lncv[0])
      {       
        return;
      }
    }
  }
}
