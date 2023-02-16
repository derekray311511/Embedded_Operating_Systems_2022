# mount the nfs server
portmap&
mount -o tcp 192.168.0.16:/home/ddfish/EOS/nfs_server /mnt

# Run main file
./hw1
