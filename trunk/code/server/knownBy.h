#ifndef KNOW_H 
#define KNOW_H 

#ifndef ulong
typedef unsigned long ulong;
#endif


#define getPlayerMask(a)  (1l<<(a))

class knownBy
{

 public:
	knownBy() { known = 0l; } 

	bool isKnownBy(ulong mask) { return (known & mask); }
	void setKnown(ulong mask) { known |= mask; }

// private :
 ulong  known;

};

#endif // SERVER_CELL_
