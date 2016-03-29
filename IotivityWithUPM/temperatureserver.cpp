
// Author: Sisinty Sasmita Patra

#include <functional>

#include <pthread.h>
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

#if EDISON
#include <upm/grove.h>
#endif

using namespace OC;
using namespace std;
namespace PH = std::placeholders;

class TemperatureResource
{

public:
    std::string m_name;
    int m_temperature;

    std::string m_temperatureUri;

    OCResourceHandle m_resourceHandle;
    OCRepresentation m_temperatureRep;

#if EDISON
    upm::GroveTemp* temp;
#endif

    public:
    TemperatureResource()
        :m_name("Edison temperature"), m_temperature(0), m_temperatureUri("/a/temperature"),
                m_resourceHandle(nullptr) {

        // Initialize representation
        m_temperatureRep.setUri(m_temperatureUri);

        m_temperatureRep.setValue("temperature", m_temperature);
        m_temperatureRep.setValue("name", m_name);

#if EDISON
	temp = new upm::GroveTemp(0);
   	m_temperature = temp->value();
#endif

    }

    void createResource()
    {
        std::string resourceURI = m_temperatureUri;
        std::string resourceTypeName = "oic.r.temperature";

        std::string resourceInterface = DEFAULT_INTERFACE;

        uint8_t resourceProperty;
        resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

        EntityHandler cb = std::bind(&TemperatureResource::entityHandler, this, PH::_1);

        OCStackResult result = OCPlatform::registerResource(
                                    m_resourceHandle, resourceURI, resourceTypeName,
                                    resourceInterface, cb, resourceProperty);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation was unsuccessful\n";
        }
    }

    // Puts representation.
    // Gets values from the representation and
    // updates the internal state
    void put(OCRepresentation& rep)
    {
        try {
            if (rep.getValue("temperature", m_temperature))
            {
                cout << "\t\t\t\t" << "temperature: " << m_temperature << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "temperature not found in the representation" << endl;
            }
        }
        catch (exception& e)
        {
            cout << e.what() << endl;
        }

    }

    // gets the updated representation.
    // Updates the representation with latest internal state before
    // sending out.
    OCRepresentation get()
    {

#if EDISON
	m_temperature = temp->value();    
#endif
        m_temperatureRep.setValue("temperature", m_temperature);

        return m_temperatureRep;
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
			// Update the lightResource
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
        TemperatureResource myTemperature;

        myTemperature.createResource();
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
