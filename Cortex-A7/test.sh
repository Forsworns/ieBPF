stty -onlcr -echo -F /dev/ttyRPMSG0
cat /dev/ttyRPMSG0 &
echo "JIT" > /dev/ttyRPMSG0
echo "INTERPRET"> /dev/ttyRPMSG0
