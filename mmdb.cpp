#include "mmdb.h"
#include "util.h"

#include <filesystem>
#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <string_view>
#include <algorithm>
#include <iterator>

#include <bitset>

std::vector<char> mmdb::readFile( const std::filesystem::path& path )
{
  if ( ! std::filesystem::is_regular_file(path) )
		throw std::runtime_error("Failed to load file: " + std::string(path));

	std::ifstream file{ path, std::ios::in | std::ios::binary };

	if ( ! file )
		throw std::runtime_error("Failed to load file: " + std::string(path));

	file.seekg(0, std::ios::end); // Seek to end of file
	std::streamoff file_len = file.tellg(); // Get the length
	file.seekg(0, std::ios::beg); // Seek to begining of file
	std::vector<char> file_blob(file_len);
	file.read(file_blob.data(), file_len);
	file.close();
	return file_blob;
}

mmdb::mmdb( const std::filesystem::path& path )
{

  // load mmdb data from file
  mmdb_data = readFile(path);
  
  // get metadata offset
  metadata_offset = findMetadataOffset();

  std::cout << "size: " << mmdb_data.size() << std::endl;
  std::cout << "metadata_offset: " << metadata_offset << std::endl;
  
  // extract important metadata
  mmdb_dtype dtype = dataFormat(metadata_offset);
	if ( (unsigned)dtype.type != 7 )
		throw std::runtime_error("Failed to find metadata map in mmdb");

  size_t offset = dtype.dataOffset;
  for ( unsigned i=0; i<dtype.size; ++i ){

    // key string
    mmdb_dtype key = dataFormat(offset);
    offset = key.nextPtr;
    printData(key);

    // value string
    mmdb_dtype val = dataFormat(offset);
    offset = val.nextPtr;
    printData(val);

    // string of key
    std::string key_str;
    if ( key.type == 2 ){
      key_str = {mmdb_data.begin()+key.dataOffset, mmdb_data.begin()+key.dataOffset+key.size};
    }

    // node_count
    if ( key.type == 2 && "node_count" == key_str ){
      uint32_t dst = 0;
      // big endian to little endian bytes
      for (size_t i=0; i < val.size; ++i )
          memcpy((char*)&dst + i, &mmdb_data[0] + val.dataOffset + val.size - i - 1, 1);
      std::cout << "node_count: " << dst << std::endl;
      node_count = dst;
    }

    // record_size(bits) converted to bytes
    if ( key.type == 2 && "record_size" == key_str ){
      uint16_t dst = 0;
      // big endian to little endian bytes
      for (size_t i=0; i < val.size; ++i )
          memcpy((char*)&dst + i, &mmdb_data[0] + val.dataOffset + val.size - i - 1, 1);
      record_size = dst / 8;
      std::cout << "record_size: " << record_size << std::endl;
    }

  }

  // set mmdb file specifications
  node_size = record_size*2;
  std::cout << "node_size: " << node_size << std::endl;
  tree_size = node_size * node_count;
  data_offset = tree_size;
  if ( data_offset > mmdb_data.size() ){
    std::cerr << "data_offset is greater than file mmdb file size" << std::endl;
    return;
  }
  record_count = node_count*2;
  if ( record_count > mmdb_data.size() ){
    std::cerr << "record_count is greater than file mmdb file size" << std::endl;
    return;
  }

  return;
}

mmdb::~mmdb()
{
}

// Find metadata offset
std::string mmdb::getCountryCodeData( size_t offset )
{

  //std::cout << "Searching data at offset " << (size_t)data_offset << std::endl;
  //std::cout << "Searching offset " << (size_t)offset << std::endl;
  // Get data
  mmdb_dtype record_data = dataFormat( data_offset + offset );
  //printData(record_data);
  //std::cout << "Found data type " << (unsigned)record_data.type << std::endl;

  std::string code = "";

  // expect map
  if ( record_data.type != 7 )
    return code;

  // iterate through map
  {
    size_t offset = record_data.dataOffset;
    for ( unsigned i=0; i<record_data.size; ++i ){

      // key string
      mmdb_dtype key = dataFormat( offset );
      offset = key.nextPtr;
      // value string
      mmdb_dtype val = dataFormat( offset );
      offset = val.nextPtr;
      
      // continent
      if ( key.type == 2 && 0 == memcmp("continent", &mmdb_data[0] + key.dataOffset, 9) ){
        // expect map
        if ( val.type != 7 )
          return code;

        // iterate through continent map
        size_t continent_offset = val.dataOffset;
        for ( unsigned j=0; j<val.size; ++j ){

          // key string
          mmdb_dtype continent_key = dataFormat( continent_offset );
          continent_offset = continent_key.nextPtr;
          // value string
          mmdb_dtype continent_val = dataFormat( continent_offset );
          continent_offset = continent_val.nextPtr;

          // continent code
          if ( continent_key.type == 2 && 0 == memcmp("code", &mmdb_data[0] + continent_key.dataOffset, 4) ){

            // expect string
            if ( continent_val.type != 2 )
              return code;
            // copy continent code
            std::string found{mmdb_data.begin()+continent_val.dataOffset, mmdb_data.begin()+continent_val.dataOffset+continent_val.size};
            if ( found != "" )
              code = found;
          }
        }
      }

      // country
      if ( key.type == 2 && 0 == memcmp("country", &mmdb_data[0] + key.dataOffset, 7) ){
        // expect map
        if ( val.type != 7 )
          return code;

        // iterate through country map
        size_t country_offset = val.dataOffset;
        for ( unsigned j=0; j<val.size; ++j ){

          // key string
          mmdb_dtype country_key = dataFormat( country_offset );
          country_offset = country_key.nextPtr;
          // value string
          mmdb_dtype country_val = dataFormat( country_offset );
          country_offset = country_val.nextPtr;

          // country code
          if ( country_key.type == 2 && 0 == memcmp("iso_code", &mmdb_data[0] + country_key.dataOffset, 4) ){
            // expect string
            if ( country_val.type != 2 )
              return code;
            // copy country iso code
            // return immediately if found country iso code
            std::string found{mmdb_data.begin()+country_val.dataOffset, mmdb_data.begin()+country_val.dataOffset+country_val.size};
            if ( found != "" )
              return found;
          }
        }
      }

    }
  }
  
  return code;
}


// Find metadata offset
size_t mmdb::findMetadataOffset()
{
  std::vector<char> metadata_terminator{'\xab','\xcd','\xef','M','a','x','M','i','n','d','.','c','o','m'};
  if ( auto it = std::search(mmdb_data.begin(), mmdb_data.end(), std::boyer_moore_searcher(metadata_terminator.begin(), metadata_terminator.end()) );
        it != mmdb_data.end() ) {
      return std::distance(mmdb_data.begin(), it) + metadata_terminator.size();
  }
  return 0;
}

// Find data offset -- not really used because this can be calculated from the tree size
size_t mmdb::findDataOffset( const char*& ptr, size_t size )
{
  char data_terminator[16];
  memset(&data_terminator, 0, 16);
  size_t data_terminator_size = 16;

  size_t end = size - data_terminator_size;
  size_t start = data_terminator_size;
  size_t min_metadata_offset = end - 128*1024 -1;

  size_t i = end;
  while ( i > min_metadata_offset ){
    if ( 0 == memcmp(data_terminator, ptr+i, data_terminator_size) ){
      return i + data_terminator_size;
    }
    --i;
  }

  return 0;
}


mmdb_dtype mmdb::dataFormat( size_t offset )
{

  if ( offset > mmdb_data.size() )
    throw std::runtime_error("offset requested is outside the file");

  // check pointer location exceeds the size of the data section

  size_t hb, s;
  uint8_t fb, sb;
  hb = 1; // header bytes
  fb = mmdb_data[offset];
  fb = fb >> 5;

  /*
  std::bitset<8> fbb(fb);
  std::cout << "at: " << offset << " first_3_bits: " << fbb << " = " << (unsigned)fb << std::endl;
  */

  // extended type
  if ( fb == 0 ){
    hb += 1;
    fb = mmdb_data[offset + 1] + 7;
  }

  // get next 5 bits for the size
  sb = mmdb_data[offset] & 31;
  s = 0;
  // If the 5 bits are less than 29 then the size is only between 0-28
  if ( sb < 29 ){
    s = sb;
  }
  // If the 5 bits are equal to 29 then the size is 29 + the next byte
  else if ( sb == 29 ){
    s = 29 + mmdb_data[offset + hb];
    hb += 1;
  }
  // if the 5 bits are equal to 30 then the size is 285 + the next 2 bytes (be)
  else if ( sb == 30 ){
    uint16_t b1 = mmdb_data[offset + hb] << 8;
    s = 285 + b1 + mmdb_data[offset + hb + 1];
    hb += 2;
  }
  // if the 5 bits are equal to 31 then the size is 65821 + the next 3 bytes (be)
  else if ( sb == 31 ){
    uint32_t b1 = mmdb_data[offset + hb] << 16;
    uint16_t b2 = mmdb_data[offset + hb + 1] << 8;
    s = 65821 + b1 + b2 + mmdb_data[offset + hb + 2];
    hb += 3;
  }

  // create a new dtype object
  mmdb_dtype dtype;
  dtype.type = fb;
  dtype.dataOffset = offset + hb;
  dtype.size = s;

  // handle different data types
  switch ( fb ){

      // Pointers - Point to byte offset in data portion(excluding null header bytes)     
    case 1:
      {
        //std::cout << "Pointer found at " << (unsigned)offset << std::endl;
        /*
        The size can be 0, 1, 2, or 3.
        If the size is 0, the pointer is built by appending the next byte to the last three bits to produce an 11-bit value.
        If the size is 1, the pointer is built by appending the next two bytes to the last three bits to produce a 19-bit value + 2048.
        If the size is 2, the pointer is built by appending the next three bytes to the last three bits to produce a 27-bit value + 526336.
        Finally, if the size is 3, the pointer’s value is contained in the next four bytes as a 32-bit value. In this case, the last three bits of the control byte are ignored.
        */
        uint8_t pb = mmdb_data[offset];
        uint8_t size = (pb & 24) >> 3; // bits 4 and 5
        uint8_t fbval = pb & 7; // bits 6, 7 and 8
        size_t data_ptr_offset = 0;

        /*
        std::bitset<8> fbb(pb);
        std::cout << "pb: " << fbb << std::endl;
        std::cout << "offset: " << offset << std::endl;
        std::cout << "size: " << (unsigned)size << std::endl;
        std::cout << "fbval: " << (unsigned)fbval << std::endl;
        */

        switch ( size ){
          case 0:
            {
              fbval = fbval << 8;
              uint8_t b1 = mmdb_data[offset + 1];
              data_ptr_offset = fbval + b1;
              dtype.nextPtr = offset + hb + 1;
            }
            break;

          case 1:
            {
              fbval = fbval << 16;
              uint16_t b1 = mmdb_data[offset + 1] << 8;
              uint8_t b2 = mmdb_data[offset + 2];
              data_ptr_offset = fbval + b1 + b2 + 2048;
              dtype.nextPtr = offset + hb + 2;
            }
            break;

          case 2:
            {
              fbval = fbval << 24;
              uint32_t b1 = mmdb_data[offset + 1] << 16;
              uint16_t b2 = mmdb_data[offset + 2] << 8;
              uint8_t b3 = mmdb_data[offset + 3];
              data_ptr_offset = fbval + b1 + b3 + 526336;
              dtype.nextPtr = offset + hb + 3;
            }
            break;

          case 3:
            {
              uint32_t b1 = mmdb_data[offset + 1] << 24;
              uint32_t b2 = mmdb_data[offset + 2] << 16;
              uint16_t b3 = mmdb_data[offset + 3] << 8;
              uint32_t b4 = mmdb_data[offset + 4];
              data_ptr_offset = b1 + b2 + b3 + b4;
              dtype.nextPtr = offset + hb + 4;
            }
            break;
        }

        //std::cout << "data_offset: " << (unsigned)data_offset << " data_ptr_offset: " << (unsigned)data_ptr_offset << " size: " << (unsigned)size << std::endl;

        // Return data at pointer
        mmdb_dtype tmp_dtype = dataFormat( data_offset + data_ptr_offset + 16 );
        dtype.size = tmp_dtype.size;
        dtype.type = tmp_dtype.type;
        dtype.dataOffset = tmp_dtype.dataOffset;
      }
      break;

    // Map
    case 7:
    // Array
    case 11:
      dtype.nextPtr = offsetFromVariableLengthData(dtype);
      break;

    // All others
    default:
      dtype.nextPtr = offset + hb + s;
      break;
  }

  return dtype;

}

size_t mmdb::offsetFromVariableLengthData( mmdb_dtype &dtype )
{
  // handle different data types
  switch ( dtype.type ){
    // Map
    case 7:
      {
        size_t offset = dtype.dataOffset;
        for ( unsigned i=0; i<dtype.size; ++i ){
          mmdb_dtype key = dataFormat( offset );
          offset = key.nextPtr;
          mmdb_dtype val = dataFormat( offset );
          offset = val.nextPtr;
        }
        return offset;
      }

    // Array
    case 11:
      {
        size_t offset = dtype.dataOffset;
        for ( unsigned i=0; i<dtype.size; ++i ){
          mmdb_dtype key = dataFormat( offset );
          offset = key.nextPtr;
        }
        return offset;
      }

    // All others
    default:
      return dtype.nextPtr;
  }
}

void mmdb::printData( mmdb_dtype& dtype )
{
  
  // handle different data types
  switch( dtype.type ){

    // Pointers - Point to byte offset in data portion(excluding null header bytes)     
    case 1:
      std::cout << "Pointer?" << std::endl;
      break;

    // String
    case 2:
      {
        std::string s{mmdb_data.begin()+dtype.dataOffset, mmdb_data.begin()+dtype.dataOffset+dtype.size};
        std::cout << "String: " << s << std::endl;
      }
      break;

    // Double (64bit float)
    case 3:
      {
        double dst = 0;
        // big endian to little endian bytes
        for (size_t i=0; i < dtype.size; ++i )
            memcpy((char*)&dst + i, &mmdb_data[0] + dtype.dataOffset + dtype.size - i - 1, 1);
        std::cout << "Double: " << dst << std::endl;
      }
      break;

    // Bytes
    case 4:
      {
        char b[dtype.size];
        memcpy( &b, &mmdb_data[0] + dtype.dataOffset, dtype.size );
        std::cout << "Bytes: " << std::hex << b << std::endl;
      }
      break;

    // Unsigned Integer 16 bit
    case 5:
      {
        uint16_t dst = 0;
        // big endian to little endian bytes
        for (size_t i=0; i < dtype.size; ++i )
            memcpy((char*)&dst + i, &mmdb_data[0] + dtype.dataOffset + dtype.size - i - 1, 1);
        std::cout << "Uint16: " << dst << std::endl;
      }
      break;

    // Unsigned Integer 32 bit
    case 6:
      {
        uint32_t dst = 0;
        // big endian to little endian bytes
        for (size_t i=0; i < dtype.size; ++i )
            memcpy((char*)&dst + i, &mmdb_data[0] + dtype.dataOffset + dtype.size - i - 1, 1);
        std::cout << "Uint32: " << dst << std::endl;
      }
      break;

    // Map
    case 7:
      std::cout << ">> Map with " << (unsigned)dtype.size << " elements" << std::endl;
      {
        size_t offset = dtype.dataOffset;
        for ( unsigned i=0; i<dtype.size; ++i ){

          std::cout << i << " = " << (unsigned)offset << std::endl;

          // key string
          mmdb_dtype key = dataFormat( offset );
          offset = key.nextPtr;
          printData( key );

          std::cout << " = " << (unsigned)offset << std::endl;

          // value string
          mmdb_dtype val = dataFormat( offset );
          offset = val.nextPtr;
          printData( val );

        }
      }
      break;

    // Integer 32 bit
    case 8:
      {
        int16_t dst = 0;
        // big endian to little endian bytes
        for (size_t i=0; i < dtype.size; ++i )
            memcpy((char*)&dst + i, &mmdb_data[0] + dtype.dataOffset + dtype.size - i - 1, 1);
        std::cout << "int32: " << dst << std::endl;
      }
      break;

    // Unsigned Integer 64bit
    case 9:
      {
        uint64_t dst = 0;
        // big endian to little endian bytes
        for (size_t i=0; i < dtype.size; ++i )
            memcpy((char*)&dst + i, &mmdb_data[0] + dtype.dataOffset + dtype.size - i - 1, 1);
        std::cout << "Uint64: " << dst << std::endl;
      }
      break;

    // Unsigned Integer 128bit
    case 10:
      {
        uint128_t dst = 0;
        // big endian to little endian bytes
        for (size_t i=0; i < dtype.size; ++i )
            memcpy((char*)&dst + i, &mmdb_data[0] + dtype.dataOffset + dtype.size - i - 1, 1);
        std::cout << "Uint128: " << (unsigned)dst << std::endl;
      }
      break;

    // Array
    case 11:
      std::cout << ">> Array with " << (unsigned)dtype.size << " elements" << std::endl;
      {
        size_t offset = dtype.dataOffset;
        for ( unsigned i=0; i<dtype.size; ++i ){
          mmdb_dtype val = dataFormat( offset );
          offset = val.nextPtr;
          printData( val );
        }
      }
      break;

    // Default
    default:
      std::cout << "Unknown: " << (unsigned) dtype.type << std::endl;
      break;
  }
}
