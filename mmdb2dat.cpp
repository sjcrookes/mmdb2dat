/*
 * g++ -std=c++20 -O3 -o mmdb2dat *.cpp
 */

#include "dat.h"
#include "mmdb.h"
#include "util.h"

#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <algorithm>

#include <iostream>

void writeIPv4( mmdb& mmdb, const std::filesystem::path& dat_file_path );

int main()
{

  mmdb mmdb("GeoIP2-Country.mmdb");
  writeIPv4( mmdb, "GeoIP.dat" );

  return 0;
}

void writeIPv4( mmdb& mmdb, const std::filesystem::path& dat_file_path )
{

  std::vector<uint32_t> tree;

  size_t record = mmdb.getIpv4RecordOffset();
  size_t record_count = mmdb.getRecordCount();
  size_t node_size = mmdb.getNodeSize();
  size_t record_size = mmdb.getRecordSize();
  size_t record_offset = mmdb.getIpv4RecordOffset();
  size_t node_count = mmdb.getNodeCount();

  const auto& mmdb_data = mmdb.getData();

  // Record data
  uint32_t left_record, right_record;

  // Iterate through records until the end
  while ( record < record_count ){

    // current tree offset by nodes
    size_t tree_offset = record * node_size;

    // load left and right records
    left_record = right_record = 0;

    for (size_t i=0; i < record_size; ++i )
      memcpy((char*)&left_record + i, &mmdb_data[0] + tree_offset + record_size - i - 1, 1);

    for (size_t i=0; i < record_size; ++i )
      memcpy((char*)&right_record + i, &mmdb_data[0] + tree_offset + record_size + record_size - i - 1, 1);

    //std::cout << "Left record: " << left_record << " Right record: " << right_record << std::endl;

    // incorrect record
    if ( left_record < record_offset || right_record < record_offset ){
      //std::cout << "record l:" << left_record << " r:" << right_record << " is smaller than offset " << record_offset << std::endl;
      break;
    }

    // if this record is the final record in the tree(unknown mapping), then set to COUNTRY_BEGIN
    if ( left_record == node_count ){
      //dat_file.write_all( &self.encode( &COUNTRY_BEGIN ) );
      tree.push_back(COUNTRY_BEGIN);
      //std::cout << "COUNTRY_BEGIN" << std::endl;
    }
    else if ( left_record < node_count ){
      //println!("left_record is another node: {}", left_record);
      //dat_file.write_all( &self.encode_usize( &(left_record - record_offset) ) );
      tree.push_back(left_record - record_offset);
      //std::cout << "Another Node" << std::endl;
    }
    else if ( left_record > node_count ){
      //std::cout << "Data" << std::endl;
      //println!("left_record is data: {}", left_record);
      // Get data
      //std::cout << "Record left: " << record << " at " << left_record - node_count << std::endl;
      std::string code = mmdb.getCountryCodeData(left_record - node_count);
      //println!("codes: {} {}", continent_code, country_code);
      //std::cout << "country_or_continent_code: " << code << std::endl;
      auto it = std::find(COUNTRY_CODES.begin(), COUNTRY_CODES.end(), code);
      if ( it != COUNTRY_CODES.end() ){
        tree.push_back(COUNTRY_BEGIN + (it - COUNTRY_CODES.begin()));
      }
      else {
        tree.push_back(COUNTRY_BEGIN);
      }
    }

    // if this record is the final record in the tree(unknown mapping), then set to COUNTRY_BEGIN
    if ( right_record == node_count ){
      //dat_file.write_all( &self.encode( &COUNTRY_BEGIN ) );
      tree.push_back(COUNTRY_BEGIN);
      //std::cout << "COUNTRY_BEGIN" << std::endl;
    }
    else if ( right_record < node_count ){
      //println!("right_record is another node: {}", right_record);
      //dat_file.write_all( &self.encode_usize( &(right_record - record_offset) ) );
      tree.push_back(right_record - record_offset);
      //std::cout << "Another Node" << std::endl;
    }
    else if ( right_record > node_count ){
      //std::cout << "Data" << std::endl;
      //println!("right_record is data: {}", left_record);
      // Get data
      //std::cout << "Record right: " << record << " at " << right_record - node_count << std::endl;
      std::string code = mmdb.getCountryCodeData(right_record - node_count);
      //println!("codes: {} {}", continent_code, country_code);
      //std::cout << "country_or_continent_code: " << code << std::endl;
      auto it = std::find(COUNTRY_CODES.begin(), COUNTRY_CODES.end(), code);
      if ( it != COUNTRY_CODES.end() ){
        tree.push_back(COUNTRY_BEGIN + (it - COUNTRY_CODES.begin()));
      }
      else {
        tree.push_back(COUNTRY_BEGIN);
      }

    }

    ++record;
  }


	std::ofstream dat_file{ dat_file_path, std::ios::out | std::ios::binary };

	if ( ! dat_file )
		throw std::runtime_error("Failed to load file: " + std::string(dat_file_path));

  // write Tree
  for ( uint32_t& node : tree){
    char* c = static_cast<char*>(static_cast<void*>(&node));
	  dat_file.write(c, 1);
	  dat_file.write(c+1, 1);
	  dat_file.write(c+2, 1);
  }

  // Comment
	dat_file.write("\x00\x00\x00", 3);
  std::string comment = "Converted with mmdb2dat by Warp Speed Computers - https://www.warp.co.nz";
	dat_file.write(comment.c_str(), comment.length());
	dat_file.write("\xff\xff\xff", 3);

  // Edition and size
	dat_file.write("\x01", 1);
  uint32_t size = record_count - record_offset;
  char* c = static_cast<char*>(static_cast<void*>(&size));
  dat_file.write(c, 1);
  dat_file.write(c+1, 1);
  dat_file.write(c+2, 1);

	dat_file.close();
}
