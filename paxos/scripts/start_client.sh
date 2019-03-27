#!/bin/sh
# Copyright (c) 2019, Tu Dang
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the project, the copyright holder nor the names of its
#  contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


if [[ -z "${LIBPAXOS_BIN}" ]]; then
  echo "${LIBPAXOS_BIN} is unset"
  exit 1
fi


sudo ${LIBPAXOS_BIN}/dppaxos/dpdk_proposer -c 3f -n 4  --socket-mem 1024 \
--file-prefix cl --log-level 7 -- --rx "(0,0,0)" --tx "(0,0)" --w "2,3,4,5" \
--pos-lb 43 --lpm "192.168.4.0/24=>0;224.0.0.103/4=>1;" --bsz "(1,1),(1,1),(1,1)" \
--multi-dbs --dst 192.168.4.98:9081 --port 0 --msgtype 8 --osd 3 --ts-interval 1 \
--src 192.168.4.95:27461 --rate 100 --node-id 1 --resp 1 --latency
