# LNMP3Player

Arduino sketch to play DFPlayer sounds based on Loconet Message

The board uses a DFPlayer (or clone) to play mp3 files stored on the sd card. The sketch used a set of Loconet addresses to trigger playing of the sounds. Loconet address #1 plays sound #1, loconet address #1+n plays sound #1+n

Programming is through lncv, lncv's are:
| lncv | default | use |
|:-:|:-:|:--|
|0 |0|not used|
|1  |1531 |article number for LNCV programming|
|2  |1 |LN address of first sound, Loconet sensor message|
|3 |4 | # of files to play|
