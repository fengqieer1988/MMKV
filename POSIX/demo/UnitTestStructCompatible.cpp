/*
 * Tencent is pleased to support the open source community by making
 * MMKV available.
 *
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *       https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MMKV.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <numeric>
#include <unistd.h>
#include <string.h>

using namespace std;
using namespace mmkv;

static const string MMKV_ID = "process_test";
static const string KeyNotExist = "KeyNotExist";

void brutleTest() {
    using hclock = chrono::high_resolution_clock;
    auto start = hclock::now();

    auto mmkv = MMKV::mmkvWithID(MMKV_ID, MMKV_MULTI_PROCESS);

    auto finish = hclock::now();
    auto used = chrono::duration_cast<chrono::milliseconds>(finish - start).count();
    cout << "used: " << used << " ms\n";
    cout.flush();
}


struct Employee
{
    int id;
    int age;
    int tall;
    int familyMemberCnt;
};
 
struct Company
{
    Employee employeeList[64]; // Employee is a struct within the Company struct
    int numberOfEmployees;
};

void testBytes(MMKV *mmkv) {
    string str = "Hello 2018 world cup 世界杯";
    MMBuffer buffer((void *) str.data(), str.length(), MMBufferNoCopy);
    auto ret = mmkv->set(buffer, "bytes");
    assert(ret);

    auto value = mmkv->getBytes("bytes");
    assert(value.length() == buffer.length() && memcmp(value.getPtr(), buffer.getPtr(), value.length()) == 0);

    value = mmkv->getBytes(KeyNotExist);
    assert(value.length() == 0);

    printf("test bytes: passed\n");
}

void testStruct1(MMKV *mmkv){
    Company company;
    company.employeeList[0].id = 100;
    company.employeeList[0].age = 101;
    company.employeeList[0].tall = 102;

    company.employeeList[1].id = 200;
    company.employeeList[1].age = 201;
    company.employeeList[1].tall = 202;

    company.numberOfEmployees = 2;

    MMBuffer buffer((void *) &company, sizeof(company), MMBufferNoCopy);

    auto ret = mmkv->set(buffer, "myStruct");
    assert(ret);

    printf("set myStruct suc, size:%d\n", sizeof(company));

    auto value = mmkv->getBytes("myStruct");
    assert(value.length() == buffer.length() && memcmp(value.getPtr(), buffer.getPtr(), value.length()) == 0);

    printf("getBytes myStruct suc, size:%d\n", sizeof(company));

    Company* readCompany = (Company*)value.getPtr();
    if(readCompany)
    {
        printf("getBytes myStruct suc, element cnt:%d\n", readCompany->numberOfEmployees);
        printf("getBytes myStruct suc, element0, id:%d, age:%d, tall:%d\n", readCompany->employeeList[0].id, readCompany->employeeList[0].age, readCompany->employeeList[0].tall);
        printf("getBytes myStruct suc, element1, id:%d, age:%d, tall:%d\n", readCompany->employeeList[1].id, readCompany->employeeList[1].age, readCompany->employeeList[1].tall);
    }
}

void testStruct2(MMKV *mmkv){

    auto readV = mmkv->getBytes("myStruct");

    printf("getBytes myStruct suc1, size:%d\n", sizeof(Company));

    Company* company = (Company*)readV.getPtr();
    if(!company)
    {
        printf("getBytes myStruct getPtr return null\n");
        return ;
    }

    printf("getBytes myStruct suc, element cnt:%d\n", company->numberOfEmployees);
    printf("getBytes myStruct suc, element0, id:%d, age:%d, tall:%d\n", company->employeeList[0].id, company->employeeList[0].age, company->employeeList[0].tall);
    printf("getBytes myStruct suc, element1, id:%d, age:%d, tall:%d\n", company->employeeList[1].id, company->employeeList[1].age, company->employeeList[1].tall);

    // 对新增的成员赋值
    company->employeeList[0].familyMemberCnt = 5;
    company->employeeList[0].familyMemberCnt = 10;

    MMBuffer buffer((void *) &company, sizeof(company), MMBufferNoCopy);

    auto ret = mmkv->set(buffer, "myStruct");
    assert(ret);

    printf("set myStruct suc, size:%d\n", sizeof(company));

    auto value = mmkv->getBytes("myStruct");
    assert(value.length() == buffer.length() && memcmp(value.getPtr(), buffer.getPtr(), value.length()) == 0);

    printf("getBytes myStruct suc2, size:%d\n", sizeof(company));

    Company* readCompany = (Company*)value.getPtr();
    if(readCompany)
    {
        printf("getBytes myStruct suc, element cnt:%d\n", readCompany->numberOfEmployees);
        printf("getBytes myStruct suc, element0, id:%d, age:%d, tall:%d\n", readCompany->employeeList[0].id, readCompany->employeeList[0].age, readCompany->employeeList[0].tall, readCompany->employeeList[0].familyMemberCnt);
        printf("getBytes myStruct suc, element1, id:%d, age:%d, tall:%d\n", readCompany->employeeList[1].id, readCompany->employeeList[1].age, readCompany->employeeList[1].tall, readCompany->employeeList[0].familyMemberCnt);
    }

}

int main() {
    locale::global(locale(""));
    wcout.imbue(locale(""));
    char c;
    srand((uint64_t) &c);

    string rootDir = "./structCompatibleTest";
    MMKV::initializeMMKV(rootDir);

    auto mmkv = MMKV::mmkvWithID("unit_test");
    mmkv->clearAll();

    testStruct2(mmkv);
}
