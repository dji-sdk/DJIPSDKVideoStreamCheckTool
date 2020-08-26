# DJIPSDKVideoStreamCheckTool

## Introduction
PSDK video stream check tool, used to check whether the specified video stream conforms to the PSDK video stream 
standard. The tool support UDP transmission or file as input, and output check result. Please refer to 
[DJI developer site](https://developer.dji.com/document/65de35b8-78ff-4199-b3ab-ce57ea95301b) for details of the PSDK 
video steam standard.

## Build Steps
1. `cd` to DJIPSDKVideoStreamCheckTool/project directory;
2. use `mkdir build` command to create a directory;
3. `cmake ..`
4. `make`

## Usage
Please get help and usage by run `./stream_check_tool -h` command in Linux terminal. Just like

```shell script
dji@manifold2:~/Documents/DJIPSDKVideoStreamCheckTool/project/build$ ./stream_check_tool -h

stream_check_tool usage:
./stream_check_tool <-u [-p <port>] [-t <type>] |-f [-i <filename>] [-t <type>] > [-h] [-v]

-t type: Stream type, can be any value of "DJI-H264" or "Custom-H264"

sample:
case 1: video stream received by UDP transmission channel as input, and please ensure every frame end with AUD (0x00 0x00 0x00 0x01 0x09 0x10)
   stream_check_tool -u -p udp_port -t stream_type
       udp_port is the UDP port which send video stream to, and the default value is 45654

case 2: video file as input
       stream_check_tool -f -i filename -t stream_type
```

#### Video File as Input 
Use below command to check local video file, and you will get output as below.

```shell script
dji@manifold2:~/Documents/DJIPSDKVideoStreamCheckTool/project/build$ ./stream_check_tool -f -i test.h264 -t DJI-H264

[Passed] 0.stream size(reference 7.3.2.1.1)
[Passed] 1.profile_idc (reference 7.3.2.1.1)
[Passed] 2.level_idc (reference 7.3.2.1.1)
[Passed] 3.YUV420 chroma_format_idc (reference 7.3.2.1.1)
[Passed] 4.chroma and luma only allow 8 bit (reference 7.3.2.1.1)
[Passed] 5.seq_scaling_matrix_presenst_flag (reference 7.3.2.1.1 and 7.3.2.2)
[Passed] 6.frame_mbs_only_flag (reference 7.3.2.1.1)
[Passed] 7.slice_type (reference 7.3.3)
[Passed] 8.num_slice_groups_minus1 (reference 7.3.2.2)
[Passed] 9.max_num_ref_frames (reference 7.3.2.1.1)
[Passed] 10.video resolution (reference 7.3.2.1.1)

[Passed] Currently, all frames has passed the above standards test.
```

#### UDP Transmission as Input
Ensure that the destination address of the video stream is 127.0.0.1 and the port can be customized such as 23003. 
Use below command to check video stream from UDP transmission, and you will get output as below.

```shell script
dji@manifold2:~/Documents/DJIPSDKVideoStreamCheckTool/project/build$ ./stream_check_tool -u -p 23003 -t DJI-H264
udp reader mode, port 23003

[Passed] 0.stream size(reference 7.3.2.1.1)
[Passed] 1.profile_idc (reference 7.3.2.1.1)
[Passed] 2.level_idc (reference 7.3.2.1.1)
[Passed] 3.YUV420 chroma_format_idc (reference 7.3.2.1.1)
[Passed] 4.chroma and luma only allow 8 bit (reference 7.3.2.1.1)
[Passed] 5.seq_scaling_matrix_presenst_flag (reference 7.3.2.1.1 and 7.3.2.2)
[Passed] 6.frame_mbs_only_flag (reference 7.3.2.1.1)
[Passed] 7.slice_type (reference 7.3.3)
[Passed] 8.num_slice_groups_minus1 (reference 7.3.2.2)
[Passed] 9.max_num_ref_frames (reference 7.3.2.1.1)
[Passed] 10.video resolution (reference 7.3.2.1.1)

[Passed] Currently, all frames has passed the above standards test.
```
