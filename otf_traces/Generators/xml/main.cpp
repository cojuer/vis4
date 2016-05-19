#include <QCoreApplication>

#include "fstream"
#include "iostream"

using namespace std;

int main(int argc, char *argv[])
{
    ofstream myfile;
    myfile.open ("trace.xml");
    myfile << "<trace max_time=\"20001\">";
    for (int j = 0; j < 10; ++j)
    {
        myfile << "   <component name=\"proc\">";
        myfile << "     <events>";
        for (int i = 0; i < 100000; ++i)
        {
            myfile << "        <event time=\"" << i << "\" letter=\"E\" kind=\"Enter\"/>";
            if (i % 1000 == 0)
                myfile << "        <group time=\"" << i << "\" target_time=\"" << i + 1 << "\" target_component=\"" << 9 - j <<"\"/>";
            myfile << "        <event time=\"" << (i + 1) << "\" letter=\"L\" kind=\"Leave\"/>";
        }
        myfile << "     </events>";
        myfile << "</component>";
    }
    myfile << "</trace>";
    return 0;
}

