cmd_/home/pi/sign/testing/crypto_module.ko := ld -EL -r  -T ./scripts/module-common.lds --build-id  -o /home/pi/sign/testing/crypto_module.ko /home/pi/sign/testing/crypto_module.o /home/pi/sign/testing/crypto_module.mod.o