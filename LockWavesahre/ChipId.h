
uint32_t chipId = 0;

uint32_t getChipId() {
  
  if (chipId == 0) {
    for(int i=0; i<17; i=i+8) {
      chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;   
    }
  }
  
  return chipId;
}

String getChipIdString() {
  return String(getChipId());
}