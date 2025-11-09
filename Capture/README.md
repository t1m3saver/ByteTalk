#采样模块
windows:
ffmpeg -f dshow -i video="BisonCam,NB Pro" -f mpegts udp://192.168.56.101:8554

linux:
sudo apt install ffmpeg v4l2loopback-dkms v4l2loopback-utils
sudo modprobe v4l2loopback devices=1 video_nr=0 card_label="VirtualCam"
ffmpeg -i udp://0.0.0.0:8554 -f v4l2 /dev/video0
