# pscap

Tool to retrieve informations about the capabilities of a process.

Inspired by the command `capshow` in the following video : Gerlof Langeveld - Practical use of Linux capabilities (Full Talk) , at the ORNL CentOS Dojo
(https://www.youtube.com/watch?v=WYC6DHzWzFQ)

## Installation

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

```bash
Usage: pscap [OPTION]... [PID]
Print process capabilities, default print current PID

OPTIONS :
  -H  --human       print human readable
  -a, --all         print all process
  -l, --line        print on one line
  -h, --help        print all group IDs
  -v, --version     print version
```

Display capabilities of current process :

```bash
pid: 5769        tid: 5769
cmd: zsh
CapEff: 0x0000000000000000
CapPrm: 0x0000000000000000
CapInh: 0x0000000000000000
CapAmb: 0x0000000000000000
CapBnd: 0x000001ffffffffff
```

Display capabilities of one process :

```bash
$ pscap 1234
pid: 1234   tid: 1234
cmd: bash
CapEff:	0000000000001000
CapPrm:	0000000000001000
CapInh:	0000000000000000
CapAmb:	0000000000000000
CapBnd:	000001ffffffffff
```

Diplay cababilities human readable of one process :

```bash
$ pscap -pH 1234
pid: 1234   tid: 1234
cmd: bash
CapEff:	CAP_NET_ADMIN
CapPrm:	CAP_NET_ADMIN
CapInh:	NONE
CapAmb:	NONE
CapBnd:	ALL
```

Display capabilities of one process (one line) :

```bash
$ pscap -l 5769
PID     TID     COMMAND     EFFECTIVE               PERMITTED               INHERITABL              AMBIENT                 BOUND
5769    5769    zsh         0x0000000000000000      0x0000000000000000      0x0000000000000000      0x0000000000000000      0x000001ffffffffff
```


Display capabilities of all process :

```bash
$ pscap -la
PID     TID     COMMAND     EFFECTIVE               PERMITTED               INHERITABL              AMBIENT                 BOUND
10804   10804   Web Con     0x0000000000000000      0x0000000000000000      0x0000000000000000      0x0000000000000000      0x000001ffffffffff
10875   10875   RDD Pro     0x0000000000000000      0x0000000000000000      0x0000000000000000      0x0000000000000000      0x000001ffffffffff
10893   10893   kworker     0x000001ffffffffff      0x000001ffffffffff      0x0000000000000000      0x0000000000000000      0x000001ffffffffff
```
