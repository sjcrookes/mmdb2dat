#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

struct mmdb_dtype
{
  uint8_t type; // type
  size_t dataOffset; // offset to data
  size_t size; // size
  size_t nextPtr; // next ptr
};

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

class mmdb
{

public:

  mmdb( const std::filesystem::path& path );
  ~mmdb();
  std::vector<char> readFile( const std::filesystem::path& path );
  std::vector<char>& getData() { return mmdb_data; };
  size_t findMetadataOffset();
  size_t findDataOffset( const char*& mmdb_ptr, size_t size );
  mmdb_dtype dataFormat( size_t offset );
  size_t offsetFromVariableLengthData( mmdb_dtype &dtype );
  size_t getMetadataOffset(){ return metadata_offset; };
  size_t getDataOffset(){ return data_offset; };
  size_t getIpv4RecordOffset(){ return ipv4_record_offset; };
  size_t getRecordCount(){ return record_count; };
  size_t getNodeSize(){ return node_size; };
  size_t getRecordSize(){ return record_size; };
  size_t getNodeCount(){ return node_count; };
  void printData( mmdb_dtype& dtype );
  std::string getCountryCodeData( size_t offset );

private:

  std::vector<char> mmdb_data;

  size_t metadata_offset = 0;
  size_t data_offset = 0;

  size_t node_count = 0;
  size_t record_size = 0;
  size_t node_size = 0;
  size_t tree_size = 0;
  size_t record_count = 0;

  // IPv4 in IPv6 Tree can skip first 96 nodes
  size_t ipv4_record_offset = 96;

};
