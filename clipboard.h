//Walt Woods
//March 9th, 2008

namespace seashell
{

namespace clipboard
{

//Standard clipboard operations

/**Sets the text to the specified string.
  * @param str String used to set the text field. */
void setText(const std::string& str);

/** @return If the clipboard is not empty, returns the associated text.  
  *Otherwise, returns an empty string. */
std::string getText();

} //clipboard

} //seashell
