#include "dat.h"

/**

void dat::write(const char* dat_file_path)
{

            let p = record * node_size;
            let left_bytes: Vec<u8> = mmdb[p..p+record_size].to_vec();
            let right_bytes: Vec<u8> = mmdb[p+record_size..p+node_size].to_vec();
            let left_record: usize = u32::from_be_bytes(pad_bytes(left_bytes, 4, true, 0).try_into().expect("Failed to load u32")).try_into().unwrap();
            let right_record: usize = u32::from_be_bytes(pad_bytes(right_bytes, 4, true, 0).try_into().expect("Failed to load u32")).try_into().unwrap();
            if left_record < record_offset || right_record < record_offset {
                println!("record {} - l:{} r:{} is smaller than offset {}", record, left_record, right_record, record_offset);
                break;
            }
            if left_record == node_count {
                //println!("left_record unknown: {}", left_record);
                dat_file.write_all( &self.encode( &COUNTRY_BEGIN ) );
            }
            else if left_record < node_count {
                //println!("left_record is another node: {}", left_record);
                dat_file.write_all( &self.encode_usize( &(left_record - record_offset) ) );
            }
            else if left_record > node_count {
                //println!("left_record is data: {}", left_record);
                // Get data
                let (continent_code, country_code) = self.get_node_data(&mmdb, left_record - node_count, data_offset);
                //println!("codes: {} {}", continent_code, country_code);
                
                let mut country_or_continent_code = country_code;
                if country_or_continent_code == "" {
                    country_or_continent_code = continent_code;
                }
                // find country code in preset list
                if let Some( index ) = COUNTRY_CODES.iter().position(|&x| x == country_or_continent_code) {
                    //println!("leaf at {} for {} ({})", index, country_or_continent_code, COUNTRY_BEGIN + index as u32);
                    dat_file.write_all( &self.encode( &(COUNTRY_BEGIN + index as u32) ) );
                }
                else {
                    //println!("leaf unknown for {} ({})", country_or_continent_code, COUNTRY_BEGIN);
                    dat_file.write_all( &self.encode( &COUNTRY_BEGIN ) );
                }
            }
            if right_record == node_count {
                //println!("right_record unknown: {}", right_record);
                dat_file.write_all( &self.encode( &COUNTRY_BEGIN ) );
            }
            else if right_record < node_count {
                //println!("right_record is another node: {}", right_record);
                dat_file.write_all( &self.encode_usize( &(right_record - record_offset) ) );
            }
            else if right_record > node_count {
                //println!("right_record is data: {}", right_record);
                // Get data
                let (continent_code, country_code) = self.get_node_data(&mmdb, right_record - node_count, data_offset);
                //println!("codes: {} {}", continent_code, country_code);
                
                let mut country_or_continent_code = country_code;
                if country_or_continent_code == "" {
                    country_or_continent_code = continent_code;
                }
                // find country code in preset list
                if let Some( index ) = COUNTRY_CODES.iter().position(|&x| x == country_or_continent_code) {
                    //println!("leaf at {} for {} ({})", index, country_or_continent_code, COUNTRY_BEGIN + index as u32);
                    dat_file.write_all( &self.encode( &(COUNTRY_BEGIN + index as u32) ) );
                }
                else {
                    //println!("leaf unknown for {} ({})", country_or_continent_code, COUNTRY_BEGIN);
                    dat_file.write_all( &self.encode( &COUNTRY_BEGIN ) );
                }
            }
            //let left_addr: std::net::Ipv4Addr = std::net::Ipv4Addr::from(left_record);
            //println!("left_addr: {}", left_addr.to_string());
            //let right_addr: std::net::Ipv4Addr = std::net::Ipv4Addr::from(right_record);
            //println!("right_addr: {}", right_addr.to_string());
            //println!("");
            record += 1;
        }

        // Comment
        dat_file.write_all(&[0x00, 0x00, 0x00]);
        dat_file.write_all(b"Converted with mmdb2dat by Warp Speed Computers - https://www.warp.co.nz");  // .dat file comment - can be anything
        dat_file.write_all(&[0xff, 0xff, 0xff]);
        // Edition and size
        dat_file.write_all(&[self.edition]);
        dat_file.write_all(&self.encode_usize( &(record_count - record_offset) ));
        println!("{:02X?}", record_count - record_offset);
        // ff ff ff 01 79 15 06
        // ff ff ff 01 00 00 00 81 15 00 00
        // ff ff ff 01 81 15 00 00
        match dat_file.flush() {
            Ok(dat_file) => dat_file,
            Err(dat_file) => println!("Error flushing data to dat file")
        }

}

*/