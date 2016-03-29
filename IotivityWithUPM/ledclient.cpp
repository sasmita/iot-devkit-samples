
// Author :Sisinty Sasmita Patra

#include <string>
#include <map>
#include <cstdlib>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;


std::shared_ptr<OCResource> curResource;
std::mutex curResourceLock;

class Led
{
public:

    bool m_value;
    std::string m_name;

    Led() : m_value(false), m_name("")
    {
    }
};

Led myled;

// callback handler on PUT request
void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK)
        {
            std::cout << "PUT request was successful" << std::endl;

            rep.getValue("value", myled.m_value);
            rep.getValue("name", myled.m_name);

            std::cout << "\tvalue: " << myled.m_value << std::endl;
            std::cout << "\tname: " << myled.m_name << std::endl;
        }
        else
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPut" << std::endl;
    }
}

// Local function to put a different value for this resource
void putLedRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;

        std::cout << "Putting led representation..."<<std::endl;

        myled.m_value = !myled.m_value;

        rep.setValue("value", myled.m_value);

        // Invoke resource's put API with rep, query map and the callback parameter
        resource->put(rep, QueryParamsMap(), &onPut);
    }
}

// Callback handler on GET request
void onGet(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK)
        {
            std::cout << "GET request was successful" << std::endl;
            std::cout << "Resource URI: " << rep.getUri() << std::endl;

            rep.getValue("value", myled.m_value);
            rep.getValue("name", myled.m_name);

            std::cout << "\tvalue: " << myled.m_value << std::endl;
            std::cout << "\tname: " << myled.m_name << std::endl;

            putLedRepresentation(curResource);
        }
        else
        {
            std::cout << "onGET Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onGet" << std::endl;
    }
}

// Local function to get representation of led resource
void getLedRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        std::cout << "Getting Led Representation..."<<std::endl;
        // Invoke resource's get API with the callback parameter

        QueryParamsMap test;
        resource->get(test, &onGet);
    }
}

// Callback to found resources
void foundResource(std::shared_ptr<OCResource> resource)
{
    std::cout << "In foundResource\n";
    std::string resourceURI;
    std::string hostAddress;
    try
    {
        {
            std::lock_guard<std::mutex> lock(curResourceLock);

            if(curResource)
            {
                std::cout << "Found another resource, ignoring"<<std::endl;
                return;
            }
        }

        // Do some operations with resource object.
        if(resource)
        {
            std::cout<<"DISCOVERED Resource:"<<std::endl;
            // Get the resource URI
            resourceURI = resource->uri();
            std::cout << "\tURI of the resource: " << resourceURI << std::endl;

            // Get the resource host address
            hostAddress = resource->host();
            std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

            // Get the resource types
            std::cout << "\tList of resource types: " << std::endl;
            for(auto &resourceTypes : resource->getResourceTypes())
            {
                std::cout << "\t\t" << resourceTypes << std::endl;
            }

            // Get the resource interfaces
            std::cout << "\tList of resource interfaces: " << std::endl;
            for(auto &resourceInterfaces : resource->getResourceInterfaces())
            {
                std::cout << "\t\t" << resourceInterfaces << std::endl;
            }

            if(resourceURI == "/a/led")
            {
                curResource = resource;
                // Call a local function which will internally invoke get API on the resource pointer
                getLedRepresentation(resource);
            }
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }

    }
    catch(std::exception& e)
    {
        std::cerr << "Exception in foundResource: "<< e.what() << std::endl;
    }
}

static FILE* client_open(const char* /*path*/, const char *mode)
{
    return fopen("./oic_svr_db_client.json", mode);
}

int main(int argc, char* argv[]) {

    std::ostringstream requestURI;

    OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };

    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Both,
        "0.0.0.0",
        0,
        OC::QualityOfService::LowQos,
        &ps
    };

    OCPlatform::Configure(cfg);

    try
    {
        std::cout.setf(std::ios::boolalpha);

        // Find all resources
        requestURI << OC_RSRVD_WELL_KNOWN_URI << "?rt=oic.r.switch.binary";

        OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, &foundResource);

        std::cout<< "Finding Resource... " <<std::endl;

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock);

    }catch(OCException& e)
    {
        oclog() << "Exception in main: "<<e.what();
    }

    return 0;
}
