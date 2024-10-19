
#ifndef GBEMU_SRC_APP_APPUTILS_H_
#define GBEMU_SRC_APP_APPUTILS_H_

#include <filesystem>
#include <cereal/cereal.hpp>

// add these functions to be able to serialize std::filesystem::path objects as strings
namespace std
{
    namespace filesystem
    {
        template<class Archive>
        void CEREAL_LOAD_MINIMAL_FUNCTION_NAME(const Archive& /*ar*/, path& out, const string& in)
        {
            out = in;
        }

        template<class Archive>
        string CEREAL_SAVE_MINIMAL_FUNCTION_NAME(const Archive& /*ar*/, const path& p)
        {
            return p.string();
        }
    }
}



#endif // GBEMU_SRC_APP_APPUTILS_H_
