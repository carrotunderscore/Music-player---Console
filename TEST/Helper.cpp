#include <vector>
#include <algorithm> // for std::transform


template <typename T>
class Helper {
private:
    

public:
    // Default constructor
    Helper() {}

    static std::vector<short> convertToShort(const std::vector<double>& input) {
        std::vector<short> output;
        output.reserve(input.size());

        for (double val : input) {
            // Clip the values to fit into short
            val = std::max(-32768.0, std::min(32767.0, val));

            // Round to nearest integer and convert to short
            output.push_back(static_cast<short>(round(val)));
        }

        return output;
    }

   

    // Any other helper functions can be added here
};
