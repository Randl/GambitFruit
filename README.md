Gambit Fruit is a free open source project based on Fruit 2.1 and Toga. The goal of Gambit Fruit is a fun free and aggresive chess engine.
To turn off Lazy Eval set Chess knowledge to 500.
If you have any suggestions please email them to me.
Bitbases can be found [here](http://oics.olympuschess.com/tracker/index.php) and latest egbbdll [here](https://sites.google.com/site/dshawul/home) or [here](https://github.com/dshawul/egbbdll)

Added:
* 6-men EGBB

TODO:
* SMP([Lazy SMP](https://chessprogramming.wikispaces.com/Parallel+Search#Shared%20Hash%20Table-Lazy%20SMP));
* Bugs and warning fixes. Static analysis with [cppcheck](http://cppcheck.sourceforge.net/);
* Converting to c++11/c++14/c++17;
* Try [Texel's Tuning Method](https://chessprogramming.wikispaces.com/Texel%27s+Tuning+Method). Using [txt automatic tool](https://bitbucket.org/zurichess/txt);
* [Progressive mobility](https://chessprogramming.wikispaces.com/Mobility#ProgressiveMobility) and [connectivity](https://chessprogramming.wikispaces.com/Connectivity);
* Kingside/queenside attack;
* [Pawn ram](https://chessprogramming.wikispaces.com/Pawn+Rams+%28Bitboards%29) handling;
* [Pawn islands](https://chessprogramming.wikispaces.com/Pawn+Islands);
* Tune extensions (King safety extension, sacrifice extension, extend on K+P eg) and reductions;
* Better draw recog(e.g. Blockage detection, KQKP);
* Values tuning (Pawn Shielding, King safety, tropism, etc);
* Typical sacs;
* [Pawn chains](https://chessprogramming.wikispaces.com/Pawn+chain);
* Custom opening book;
* [Chess960](http://chessprogramming.wikispaces.com/Chess960) support;
* Center control (With B as a bonus for a given square, this is B*[AT+2*OC] where AT is #attackers and OC is #occupants.)
* Bad bishop 


Thanks for everyone contributed to Gambit Fruit, Fruit 2.1, Toga, and to those who gave me ideas.