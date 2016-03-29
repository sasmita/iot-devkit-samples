
// Author:Sisinty Sasmita Patra

#include <functional>

#include <pthread.h>
#include <mutex>
#include <condition_variable>

#if EDISON
#include <upm/grove.h>
#endif

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;
namespace PH = std::placeholders;

class LedResource
{

public:
    std::string m_name;
    bool m_value;

    std::string m_ledUri;

    OCResourceHandle m_resourceHandle;
    OCRepresentation m_ledRep;

#if EDISON
    upm::GroveLed* led;
#endif

public:
    LedResource()
        :m_name("Edison led"), m_value(false), m_ledUri("/a/led"),
                m_resourceHandle(nullptr) {

#if EDISON
	led = new upm::GroveLed(2);
#endif

        // Initialize representation
        m_ledRep.setUri(m_ledUri);

        m_ledRep.setValue("value", m_value);
        m_ledRep.setValue("name", m_name);
    }

    void createResource()
    {
        std::string resourceURI = m_ledUri;
        std::string resourceTypeName = "oic.r.switch.binary";

        std::string resourceInterface = DEFAULT_INTERFACE;

        uint8_t resourceProperty;
        resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

        EntityHandler cb = std::bind(&LedResource::entityHandler, this, PH::_1);

        OCStackResult result = OCPlatform::registerResource(
                                    m_resourceHandle, resourceURI, resourceTypeName,
                                    resourceInterface, cb, resourceProperty);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation was unsuccessful\n";
        }
    }

    void put(OCRepresentation& rep)
    {
        try {
            if (rep.getValue("value", m_value))
            {

#if EDISON
		if(m_value == true)
			led->on();
		else
			led->off();	
#endif

                cout << "\t\t\t\t" << "value: " << m_value << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "value not found in the representation" << endl;
            }

        }
        catch (exception& e)
        {
            cout << e.what() << endl;
        }

    }

    OCRepresentation get()
    {
        m_ledRep.setValue("value", m_value);

        return m_ledRep;
    }

private:
	OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
	    OCEntityHandlerResult ehResult = OC_EH_ERROR;

	    if(request)
	    {
		// Get the request type and request flag
		std::string requestType = request->getRequestType();
		int requestFlag = request->getRequestHandlerFlag();

		if(requestFlag & RequestHandlerFlag::RequestFlag)
		{
		    cout << "\t\trequestFlag : Request\n";

		    auto pResponse = std::make_shared<OC::OCResourceResponse>();

		    pResponse->setRequestHandle(request->getRequestHandle());
		    pResponse->setResourceHandle(request->getResourceHandle());

		    // If the request type is GET
		    if(requestType == "GET")
		    {
			cout << "\t\t\trequestType : GET\n";
			pResponse->setErrorCode(200);
			pResponse->setResponseResult(OC_EH_OK);
			pResponse->setResourceRepresentation(get());
			if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
			{
			    ehResult = OC_EH_OK;
			}
		    }
		    else if(requestType == "PUT")
		    {
			cout << "\t\t\trequestType : PUT\n";
			OCRepresentation rep = request->getResourceRepresentation();

			// Do related operations related to PUT request
			// Update the ledResource
			put(rep);
			pResponse->setErrorCode(200);
			pResponse->setResponseResult(OC_EH_OK);
			pResponse->setResourceRepresentation(get());
			if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
			{
			    ehResult = OC_EH_OK;
			}
		    }
		    else if(requestType == "POST")
		    {
			cout << "\t\t\trequestType : POST\n";
		    }
		    else if(requestType == "DELETE")
		    {
			cout << "Delete request received" << endl;
		    }
		}
	    }
	    else
	    {
		std::cout << "Request invalid" << std::endl;
	    }

	    return ehResult;
	}

};

static FILE* server_open(const char* /*path*/, const char *mode)
{
    return fopen("./oic_svr_db_server.json", mode);
}

int main(int argc, char* argv[])
{
    OCPersistentStorage ps {server_open, fread, fwrite, fclose, unlink };

    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Server,
        "0.0.0.0", 
        0,         
        OC::QualityOfService::LowQos,
        &ps
    };

    OCPlatform::Configure(cfg);

    try
    {
        LedResource myLed;

        myLed.createResource();
        std::cout << "Created resource." << std::endl;

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        std::cout <<"Waiting" << std::endl;
        cv.wait(lock, []{return false;});
    }
    catch(OCException &e)
    {
        std::cout << "OCException in main : " << e.what() << endl;
    }

    // No explicit call to stop the platform.
    // When OCPlatform::destructor is invoked, internally we do platform cleanup

    return 0;
}
