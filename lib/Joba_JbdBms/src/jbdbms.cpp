#include <jbdbms.h>


// Debug

void JbdBms::hex( const char *label, const uint8_t *data ) {
    size_t length = sizeof(data);
    Serial.print(label);
    while( length-- ) {
        Serial.printf("%02X ", *(data++));
    }
    Serial.println();
}


// Basic methods

JbdBms::JbdBms( uint32_t *prev, uint8_t command_delay_ms ) 
    : _delay(command_delay_ms), _prev(prev), _dir_pin(-1) {
    if (!_prev) {
        _prev = &_prev_local;
    }
}

void JbdBms::begin( int dir_pin ) {
    _dir_pin = dir_pin;
    if( _dir_pin >= 0 ) {
        pinMode(_dir_pin, OUTPUT);
        digitalWrite(_dir_pin, LOW);  // read mode (default)
    }
}

bool JbdBms::execute( request_header_t &header, uint8_t *command, uint8_t *result ) {
    uint16_t crc;
    uint8_t stop = 0x77;

    if( !prepareCmd(header, command, crc) ) {
        return false;
    }

    uint32_t remaining = _delay - (millis() - *_prev);
    if( remaining <= _delay ) {
        delay(remaining);
    }

    if( _dir_pin >= 0 ) {
        digitalWrite(_dir_pin, HIGH);  // write mode
    }

    Serial1.flush();  // make sure read buffer is empty
    bool rc = (Serial1.write((uint8_t *)&header, sizeof(header)) == sizeof(header)) // 
           && (Serial1.write(command, header.length) == header.length)
           && (Serial1.write((uint8_t *)&crc, sizeof(crc)) == sizeof(crc))
           && (Serial1.write(&stop, sizeof(stop)) == sizeof(stop));
    Serial1.flush();  // wait until write is done 
    
    // // DEBUG
    // Serial.println("SENT HEX TO BMS: ");
    // Serial.flush();  // make sure read buffer is empty
    // hex("Header:  ", (uint8_t *)&header);
    // hex("Command: ", (uint8_t *)&command);
    // hex("CRC:     ", (uint8_t *)&crc);
    // hex("STOP:    ", (uint8_t *)&stop);
    // Serial.flush();  // wait until write is done 
    // Serial.println();
    // Serial.println("--------------------");
    // // DEBUG

    if( _dir_pin >= 0 ) {
        digitalWrite(_dir_pin, LOW);  // read mode (default)
    }

    if( rc ) {
        response_header_t header = {0};
        rc = (Serial1.readBytes((uint8_t *)&header, sizeof(header)) == sizeof(header))
          && (header.start == 0xdd && header.length <= 64)
          && (header.length == 0 || (result && Serial1.readBytes(result, header.length) == header.length))
          && (Serial1.readBytes((uint8_t *)&crc, sizeof(crc)) == sizeof(crc))
          && (Serial1.readBytes(&stop, sizeof(stop)) == sizeof(stop))
          && isValid(header, result, crc)
          && header.returncode == 0;

        // // DEBUG
        // Serial.println("READ HEX FROM BMS: ");
        // Serial.flush();  // make sure read buffer is empty
        // uint8_t *header_hex_2 = (uint8_t *)&header;
        // uint8_t *crc_hex_2 = (uint8_t *)&crc;
        // hex("Header:  ", (uint8_t *)&header);
        // hex("Result:  ", (uint8_t *)&result);
        // hex("CRC:     ", (uint8_t *)&crc);
        // hex("STOP:    ", (uint8_t *)&stop);
        // Serial.flush();  // wait until write is done 
        // Serial.println();
        // Serial.println("--------------------");
        // // DEBUG

        // pCharacteristicTx->setValue(header);
        // pCharacteristicTx->notify();
    }

    *_prev = millis();

    return rc;
}

// public Get-Commands

bool JbdBms::getStatus( Status_t &data ) {
    request_header_t header = { 0, READ, STATUS, 0 };
    bool rc = execute(header, 0, (uint8_t *)&data);
    swap(&data.voltage);
    swap((uint16_t *)&data.current);
    swap(&data.remainingCapacity);
    swap(&data.nominalCapacity);
    swap(&data.cycles);
    swap(&data.productionDate);
    swap(&data.balanceLow);
    swap(&data.balanceHigh);
    swap(&data.fault);
    return rc;
}
    
bool JbdBms::getCells( Cells_t &data ) {
    request_header_t header = { 0, READ, CELLS, 0 };
    bool rc = execute(header, 0, (uint8_t *)&data);
    for (size_t i = 0; i < sizeof(data.voltages)/sizeof(*data.voltages); i++) {
        swap(&data.voltages[i]);
    }
    return rc;
}
    
bool JbdBms::getHardware( Hardware_t &data ) {
    request_header_t header = { 0, READ, HARDWARE, 0 };
    return execute(header, 0, (uint8_t *)&data);
}


// public Set-Command

bool JbdBms::setMosfetStatus( mosfet_t status ) {
    request_header_t header = { 0, WRITE, MOSFET, 2 };
    uint8_t status_inv = ~status & MOSFET_BOTH;  // invert status pins
    uint8_t mosfetStatus[] = { 0, status_inv };
    return execute(header, mosfetStatus, 0);
}


// Private Stuff (used internally, not by library user)

// Calculate 16-bit crc of request
// Return crc (0 on error)
uint16_t JbdBms::genRequestCrc( request_header_t &header, uint8_t *data ) {
    return genCrc(header.command, header.length, data);
}

// Calculate 16-bit crc of response
// Return crc (0 on error)
uint16_t JbdBms::genResponseCrc( response_header_t &header, uint8_t *data ) {
    return genCrc(header.returncode, header.length, data);
}

uint16_t JbdBms::genCrc( uint8_t byte, uint8_t len, uint8_t *data ) {
    uint16_t crc = 0;

    if( len < 31 && (len == 0 || data)) {
        crc -= byte;
        crc -= len;
        while( len-- ) {
            crc -= *(data++);
        }
    }

    return swap(&crc);
}

// Check crc of result
// Return true if calculated and stored crc match 
bool JbdBms::isValid( response_header_t &header, uint8_t *data, uint16_t crc ) {
    return genResponseCrc(header, data) == crc;
}

// Set start and crc bytes of command
// Return length of command or 0 on errors
bool JbdBms::prepareCmd( request_header_t &header, uint8_t *data, uint16_t &crc ) {
    header.start = 0xdd;
    crc = genRequestCrc(header, data);
    return crc != 0;
}


// Convert balance bits to string
// WARNING: not thread safe: returns shared buffer
const char *JbdBms::balance( const Status_t &data ) {
    static char balanceStr[33];

    char *balancePtr = balanceStr;
    uint32_t balanceBits = (uint32_t)data.balanceHigh << 16 | data.balanceLow;
    size_t cell = (data.cells < sizeof(balanceStr)) ? data.cells : sizeof(balanceStr) - 1;
    while(cell--) {
        *(balancePtr++) = (balanceBits & 1) ? '1' : '0';
        balanceBits >>= 1;
    }
    *balancePtr = '\0';

    return balanceStr;
}
