# AECAL
Innovative console calendar based on cal.c from bsd 4.2

## Build
For compile use:
```
git clone https://github.com/Ae-foundation/aecal
cd aecal
make
```
For install use:
```
make install
```
For uninstall use:
```
make uninstall
```

## Using
```
./aecal           will display the current month of the year
./aecal 1023      will display the entire year 1023
./aecal 1023 4    will display the entire 4th month of 1023

will display the entire current year
  ./aecal -y
  ./aecal y
  ./aecal year

display the date of Easter
  ./aecal -e
  ./aecal -e 555    for the year 555

display the help menu
  ./aecal -h
  ./aecal h
  ./aecal help
```
