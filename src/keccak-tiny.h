/*
   Copyright 2019 Ethereum Foundation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef SILKWORM_KECCAK_TINY_H_
#define SILKWORM_KECCAK_TINY_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int sha3_256(uint8_t* out, size_t outlen, const uint8_t* in, size_t inlen);
int sha3_512(uint8_t* out, size_t outlen, const uint8_t* in, size_t inlen);

#ifdef __cplusplus
}
#endif

#endif  // SILKWORM_KECCAK_TINY_H_
