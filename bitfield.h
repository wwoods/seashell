//Walt Woods
//July 31, 2008

#ifndef BITFIELD_H_
#define BITFIELD_H_

namespace seashell {

//A bitfield class used in compression algorithms or special binary file 
//writing.
//
//The bitfield is stored in FIFO format (the MSB of integers are the first 
//bits, not the last)
class Bitfield {
public:
	Bitfield() { bitOut_ = BITS; field_ = 0; write_.push_back(0); }
	~Bitfield() {}

	/**Sets up the externally allocated bitfield.  Initializes reading 
	  *process.  */
	void setReadfield(suint* field, suint count) 
	{ 
		field_ = field; 
		fieldlen_ = count / (sizeof(suint) << 3) + 1;
		fieldlast_ = BITS - (count % (sizeof(suint) << 3));
		bitIn_ = BITS; 
		word_ = 0; 
	}
	
	/**Returns the bitfield used for writes. */
	suint* getWritefield(suint* len) { if (len) *len = (((suint)write_.size() * sizeof(suint)) << 3) - bitOut_; return &write_.front(); }

	/**Writes bits to the bitfield.  Returns zero on error. */
	inline sint writeBits(suint in, suint count) 
	{
		//Can't write more than one system int at a time
		if (count > BITS)
			return 0;
			
		eassert(!(in & ~((1 << count) - 1)), Exception, "in field (%i) was "
		  "expected to have (%i) bits, but had more", in, count);
			
		//Will the new bits fit in the last word?
		if (count >= bitOut_) {
			//in has [0, count) bits set.  Extract [count - bits_, count).
			count -= bitOut_;
			bitOut_ = BITS;
			write_.back() |= (in >> count);
			in &= (1 << count) - 1;
			write_.push_back(0);
		}
		
		//More to write / was there anything ever?
		if (count > 0) {
			bitOut_ -= count;
			write_.back() |= (in << bitOut_);
		}
		
		return 1;
	}
	
	/**Reads a number of bits from an external field.  
      * @return Returns (suint)-1 on EOF. */
	inline suint readBits(suint count)
	{
		eassert(field_, Exception, "Required: Bitfield must have external "
		  "field for read request");
		  
		if (word_ == fieldlen_ - 2 && count > bitIn_ + (BITS - fieldlast_) ||
		  word_ == fieldlen_ - 1 && count > bitIn_ - fieldlast_ ||
		  word_ >= fieldlen_) {
			word_ = fieldlen_;
		    return (suint)-1;
		}
		  
		register suint ret = 0;
		register suint temp = count;
		
		if (count >= bitIn_) {
			count -= bitIn_;
			bitIn_ = BITS;
			ret |= (field_[word_++] << count);
		}
		
		if (count > 0) {
			bitIn_ -= count;
			ret |= ((field_[word_] >> bitIn_) & ((1 << count) - 1));
		}
		
		//Crop ret to the right number of bits
		ret &= ((1 << temp) - 1);
		
		return ret;
	}
	
private:
	//Last bit written
	suint bitOut_;
	
	//Internal buffer for writes
	std::vector<suint> write_;
	
	//Externally allocated bitfield
	suint* field_;
	
	//Length of externally allocated bitfield
	suint fieldlen_;

    //Max bit value in the last byte of the field
    suint fieldlast_;
	
	//Current system int in external field
	suint word_;

    //Last bit read
    suint bitIn_;
};

} //seashell

#endif//BITFIELD_H_
