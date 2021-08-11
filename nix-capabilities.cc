/*
- Complile with
```
> g++ ./nix-capabilities.cc -o nix-capabilites
```
- Set CAP_CHOWN and CAP_FOWNER capabilites for `nix-capabilities`
```
> sudo setcap 'cap_chown,cap_fowner+eip' nix-capabilites
```
- Test with /nix folder
```
> ls -la /nix/
drwxr-xr-x   4 dimap dimap   4096 Jun 30 22:40 var
> ./nix-capabilities chown /nix/var 0 0
> ls -la /nix
...
drwxr-xr-x   4 root  root    4096 Jun 30 22:40 var
```
> ./nix-capabilities chmod /nix/var 1222
> ls -la /nix/
...
d-w--w--wT   4 root  root    4096 Jun 30 22:40 var
*/

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

void usage() {
    std::cerr << "USAGE:"
              << " <chown|chmod>"
              << " <path to file/directory under /nix to change ownership>"
              << " <uid|perm>"
              << " [gid]" << std::endl;
}

uint32_t getPerm(const std::string &perm) {
    return std::stoul(perm, (std::size_t *)0, 8) & 0xfff;
}

std::string getCanonicalizedPath(const char *path) {
    char* res = realpath(path, NULL);
    if (res == NULL) {
        return "";
    }
    std::string pathToFile{res};
    free(res);
    return pathToFile;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        usage();
        return -1;
    }

    std::string mode{argv[1]};
    if (mode != "chmod" && mode != "chown") {
        usage();
        return -1;
    }

    std::string pathToFile = getCanonicalizedPath(argv[2]);
    if (pathToFile.length() == 0) {
        std::cerr << "Path " << argv[2] << " doesn't exist" << std::endl;
        return -1;
    };
    if (pathToFile.rfind("/nix/", 0) != 0) {
        usage();
        return -1;
    }

    std::string uidOrPerm{argv[3]};
    if (mode == "chown") {
        if (argc != 5) {
            usage();
            return -1;
        }
        std::string gid{argv[4]};
        return chown(pathToFile.c_str(), std::stoul(uidOrPerm),
                     std::stoul(gid));
    } else if (mode == "chmod") {
        if (argc != 4) {
            usage();
            return -1;
        }
        return chmod(pathToFile.c_str(), getPerm(uidOrPerm));
    } else {
        usage();
        return -1;
    }
    return 0;
}
