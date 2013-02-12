/*
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.  You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef EMPNUMS_H
#define EMPNUMS_H

#include "ECString.h"


#define ERROREMP -1
#define GENEMP 1
#define SBAREMP 2
#define OTHEMP 3
#define UNARYEMP 4
#define QEMP 5
#define NOTEMP 6
#define RNREMP 7
#define ICHEMP 8
#define EXPEMP 9
#define PPAEMP 10
#define TREMP 11
#define NPEMP 12
#define NULLEMP 13
#define UNITEMP 14

int whichEmpty(const ECString& emp);
int swEmpty(const ECString& wrd, const ECString& trm);

ECString emptyFromInt(int emp);

#endif				/* ! EMPNUMS_H */
