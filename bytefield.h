//Walt Woods
//August 8th, 2008

#ifndef SEASHELL_BYTEFIELD_H_
#define SEASHELL_BYTEFIELD_H_

namespace seashell {

//A byte class used for writing files or other arbitrary binary chunks
class Bytefield {
public:
	Bytefield() { byteOut_ = 0; field_ = 0; }
	~Bytefield() {}

	/**Sets up the externally allocated bitfield.  Initializes reading 
	  *process.  */
	void setReadfield(suint8* field, suint length) 
	{ 
		field_ = (suint8*)field; 
		fieldlen_ = length;
		byteIn_ = 0;
	}
	
	/**Returns the bitfield used for writes. */
	suint8* getWritefield(suint* len) { if (len) *len = (suint)write_.size(); return &write_.front(); }

	/**Writes bytes to the bytefield. */
	inline Bytefield& operator<<=(suint32 data)
	{
        write_.resize(byteOut_ + sizeof(suint32));
        *(suint32*)&write_[byteOut_] = data;
        byteOut_ += sizeof(suint32);
        return *this;
	}

	/**Writes a string to the bytefield.  Do write the null terminator */
	inline Bytefield& operator<<=(const char* data)
	{
        suint len = (suint)strlen(data) + 1;

        write_.resize(byteOut_ + len);
        memcpy(&write_[byteOut_], data, len - 1);
        byteOut_ += len;
        write_[byteOut_ - 1] = 0;
        return *this;
	}

    /**Reads bytes from the bytefield. */
    inline void read(suint8* out, suint len)
    {
        if (len == 0)
            return;

        if (byteIn_ + len >= fieldlen_)
            ethrow(Exception, "Unable to read bytefield past boundary");

        memcpy(out, &field_[byteIn_], len);
        byteIn_ += len;
    }
	
private:
	//Last bit written
	suint byteOut_;
	
	//Internal buffer for writes
	std::vector<suint8> write_;
	
	//Externally allocated bitfield
	suint8* field_;
	
	//Length of externally allocated bitfield
	suint fieldlen_;

    //Last bit read
    suint byteIn_;
};

} //seashell

#endif//SEASHELL_BYTEFIELD_H_
