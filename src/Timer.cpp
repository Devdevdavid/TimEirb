#include "Timer.h"
#include "Channel.h"
#include "Tools.h"

/**
 * Public methods
 */

Timer::Timer(sc_module_name name, uint32_t baseAddress) :
              sc_module(name),
              socketPMC("socketPMC"),
              socketBus("socketBus") {
  // Set internal variables
  this->baseAddress = baseAddress;
  this->curPmcData.mck = 0;                         // Timer starts disabled by default
  this->curPmcData.slck = 0;                        // Timer starts disabled by default
  memset(this->registerData, 0, sizeof(this->registerData));  // Reset value of internal registers
  this->isWriteProtected = false;                   // Write protection is disabled at reset

  // Instanciate channels
  for (int i = 0; i < CHANNEL_COUNT; ++i) {
    string name = "Channel" + to_string(i);
    channels[i] = new Channel(name.c_str());
  }

  // Link sockets
  socketPMC.register_b_transport(this, &Timer::b_transport_pcm);
  socketBus.register_b_transport(this, &Timer::b_transport_bus);
}

void Timer::b_transport_pcm(tlm_generic_payload& trans, sc_time& delay)
{
    struct pmc_data *pmcData;

    // We support only write command on this socket
    if (trans.get_command() != TLM_WRITE_COMMAND) {
        trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
        return;
    }

    // The only data valid is a struct pmc_data, check length
    if (trans.get_data_length() != sizeof(struct pmc_data)) {
        trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
        return;
    }

    // Cast and check the pointer
    pmcData = (struct pmc_data *) trans.get_data_ptr();
    if (pmcData == NULL) {
        trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
        return;
    }

    // Check if data changed
    if (memcmp(&(this->curPmcData), pmcData, sizeof(struct pmc_data)) != 0) {
      // Save data into internal attribute
      memcpy(&(this->curPmcData), pmcData, sizeof(struct pmc_data));

      // Tell all the channels
      for (int i = 0; i < CHANNEL_COUNT; ++i) {
        channels[i]->set_pmc_clock(this->curPmcData);
      }

      // Print new frequency for debug
      cout << "Timer: new mck = " << this->curPmcData.mck << " Hz"
           << " new slck = " << this->curPmcData.slck << " Hz" << endl;
    } else {
      // Print new frequency for debug
      cout << "Timer: clocks unchanged" << endl;
    }

    trans.set_response_status(TLM_OK_RESPONSE);
}

void Timer::b_transport_bus(tlm_generic_payload& trans, sc_time& delay)
{
  bool isForTimerItSelf = true;
  uint32_t addr = trans.get_address();
  uint32_t *pData = (uint32_t *) trans.get_data_ptr();
  uint32_t cmd = trans.get_command();

  // Check if address is in our space
  if (!is_in_range(addr, this->baseAddress, TIMER_ADDR_SPACE)) {
    trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }

  // Check len: multiple register read/write not supported
  if (trans.get_data_length() != REGISTER_SIZE) {
    trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
    return;
  }

  // Remove addresse base offset
  addr -= this->baseAddress;

  for (int i = 0; i < CHANNEL_COUNT; ++i) {
    if (is_in_range(addr, i * TIMER_CHANNEL_ADDR_SPACE, TIMER_CHANNEL_ADDR_SPACE)) {
      // Remove address offset for the selected channel
      addr -= i * TIMER_CHANNEL_ADDR_SPACE;

      // Notify that this data is not for timer itself
      isForTimerItSelf = false;

      // Send data to the right channel
      if (channels[i]->manage_register(cmd, addr, pData) != 0) {
        trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
        return;
      }
      break;
    }
  }

  // To timer itself
  if (isForTimerItSelf) {
    if (manage_register(cmd, addr, pData) != 0) {
      trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
      return;
    }
  }
  trans.set_response_status(TLM_OK_RESPONSE);
}

/**
 * Private methods
 */

/**
 * @brief Manage read and write register for timer
 *
 * @param cmd TLM_WRITE_COMMAND or TLM_READ_COMMAND
 * @param address [TC_BCR to TC_WPMR]
 * @param pData Pointer on a uint32_t[1]
 * @return 0: Success, -1: Error
 */
int Timer::manage_register(uint8_t cmd, uint32_t address, uint32_t *pData)
{
  int maxfilt=0;
  switch (address) {
    case TC_BCR:                /** Block control */
      _is_write_only_();

      if ((*pData) & TC_BCR_SYNC) {
        // Sync all channels
        for (int i = 0; i < CHANNEL_COUNT; ++i) {
          //channels[i]->set_sync();//décommenter si réalisé dans channel
        }
      }
      break;

    case TC_BMR:                /** Block mode */
      if (_is_read()) {
        (*pData) = registerData[TC_BMR_I];
      } else {
        _need_wpen_();
        registerData[TC_BMR_I] = (*pData) & TC_BMR_Mask;
        //TC_BMR_TC0XC0S, TC_BMR_TC1XC1S, TC_BMR_TC2XC2S not used

        if (registerData[TC_BMR_I] & TC_BMR_QDEN){
          if ((registerData[TC_BMR_I] & TC_BMR_POSEN) || (registerData[TC_BMR_I] & TC_BMR_SPEEDEN)){
            //enable QDEC
            //Not used in waveform mode
          }
        }
        else{

        }

        if (registerData[TC_BMR_I] & TC_BMR_POSEN){
          //Not used in waveform mode
        }
        else{
          
        }
        if (registerData[TC_BMR_I] & TC_BMR_SPEEDEN){
          
        }else{
          
        }

        if (registerData[TC_BMR_I] & TC_BMR_QDTRANS){
          
        }
        else{
          
        }

        if (registerData[TC_BMR_I] & TC_BMR_EDGPHA){
          
        }
        else{
          
        }

        if (registerData[TC_BMR_I] & TC_BMR_INVA){
          
        }
        else{
          
        }

        if (registerData[TC_BMR_I] & TC_BMR_INVB){
          
        }
        else{
          
        }

        if (registerData[TC_BMR_I] & TC_BMR_INVIDX){
          
        }
        else{
          
        }

        if (registerData[TC_BMR_I] & TC_BMR_SWAP){
          
        }
        else{
          
        }
        if (registerData[TC_BMR_I] & TC_BMR_IDXPHB){

        }
        else{
          
        }
        
        maxfilt = (registerData[TC_BMR_I] & TC_BMR_MAXFILT)>>20;

      }
      break;

    case TC_QIER:               /** Enable interrupts */
      _is_write_only_();

      // Enable bits only if they are in the mask
      registerData[TC_QIMR_I] |= (*pData) & TC_QIxR_Mask;
      break;

    case TC_QIDR:               /** Disable interrupts */
      _is_write_only_();

      // Disable bits only if they are in the mask
      registerData[TC_QIMR_I] &= ~((*pData) & TC_QIxR_Mask);
      break;

    case TC_QIMR:               /** Interrupt mask */
      _is_read_only_();

      // Just read enabled interrupts
      (*pData) = registerData[TC_QIMR_I] & TC_QIxR_Mask;
      break;

    case TC_QISR:               /** Active interrupts */
      _is_read_only_();
      // Mask all disabled interrupts
      (*pData) = registerData[TC_QISR_I] & registerData[TC_QIMR_I] & TC_QIxR_Mask;
      this->registerData[TC_QISR] = 0;
      break;

    case TC_FMR:                /** Fault mode */
      if (_is_read()) {
        (*pData) = registerData[TC_FMR_I];
      } else {
        _need_wpen_();

        // Write only bits in the mask
        registerData[TC_FMR_I] = (*pData) & TC_FMR_Mask;

        //Not used for the moment
        if (registerData[TC_FMR_I] & TC_FMR_ENCF0){

        }else
        {

        }
        if (registerData[TC_FMR_I] & TC_FMR_ENCF1){

        }else{

        }
      }
      break;

    case TC_WPMR:               /** Write protection */
      if (_is_read()) {
        (*pData) = registerData[TC_WPMR_I];
      } else {
        // Save only the WPEN bit (not the password!)
        registerData[TC_WPMR_I] = (*pData) & TC_WPMR_WPEN;

        // Is the password ok ?
        if (((*pData) >> 8) == TC_WPMR_PASSWORD) {
          // Is the Write protection enabled ?
          if ((*pData) & TC_WPMR_WPEN) {
            set_write_protection(true);
          } else {
            set_write_protection(false);
          }
        }
      }
      break;

    default:
      fprintf(stderr, "Timer::manage_register() Ignoring %s at address: 0x%02X\n",
        _is_write() ? "write" : "read",
        address);
      return -1;
  }

  return 0;
}

/**
 * @brief Set the write protection on supported registers
 *
 * @param isEnabled boolean
 */
void Timer::set_write_protection(bool isEnabled)
{
  this->isWriteProtected = isEnabled;

  // Tell all the channels
  for (int i = 0; i < CHANNEL_COUNT; ++i) {
    channels[i]->set_write_protection(isEnabled);
  }

}
