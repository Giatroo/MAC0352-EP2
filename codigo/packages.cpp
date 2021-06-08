#ifndef PACKAGES_CPP
#define PACKAGES_CPP

#include "packages.hpp"

#include <stdio.h>
#include <stdlib.h>

#include "util.hpp"

FixedHeader::FixedHeader() { }
FixedHeader::FixedHeader(ustring recvline) {
    this->header_type = recvline[0];
    this->remaning_length = byte_str_to_int(recvline, 1);
}

ustring FixedHeader::write_fixed_header() {
    ustring fixed_header_str;
    fixed_header_str = (ustring) malloc(3 * sizeof(uchar));
    fixed_header_str[0] = this->header_type;
    ustring temp = int_to_2byte_str(this->remaning_length);
    fixed_header_str[1] = temp[0];
    fixed_header_str[2] = temp[1];
    free(temp);

    return fixed_header_str;
}

HeaderTemplate::HeaderTemplate() { }

ustring CreateUserPackage::header_to_string(size_t &len) {
    ustring ret_str;
    this->fixed_header.header_type = CREATE_USER_PACKAGE;

    int name_len = this->username.size();
    int pwd_len = this->password.size();
    int total_len = name_len + pwd_len + 4;
    ret_str = (ustring) malloc((3 + total_len) * sizeof(uchar));
    this->fixed_header.remaning_length = total_len;

    int i = 0;
    ustring fixed_header_str = this->fixed_header.write_fixed_header();
    for (i = 0; i < 3; ++i) { ret_str[i] = fixed_header_str[i]; }
    write_string(ret_str, i, this->username);
    write_string(ret_str, i, this->password);

    len = total_len + 3;
    return ret_str;
}

void CreateUserPackage::string_to_header(ustring recvline, size_t len) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    read_string(recvline, pos, this->username);
    read_string(recvline, pos, this->password);
}

#endif /* ifndef PACKAGES_CPP */
