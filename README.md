# killresp

killresp is a header only project that allows you to respond to signals by writing to the senders stdout the project is named killresp because it was responds to sigterm but you can easily swap this out with another signal in the code. see src/example.c for a working example all actual logic can be found in src/killresp.h

### usage

./killresp "message to write to senders stdout"

![Image](https://media.discordapp.net/attachments/1431818476116381840/1433209934031622406/eRPkgJ0.png?ex=6903dc44&is=69028ac4&hm=b7c8e661ac6b22ff92e6c368c6dd645bad91b3980fb3ede3b35a582a4327bfd9&=&format=png&quality=lossless)

