// RFC3261 19.1.1 SIP and SIPS URI Components (p148)
// RFC3261 19.1.2 Character Escaping Requirements (p152)
// sip:user:password@host:port;uri-parameters?headers
// sip:alice@atlanta.com
// sip:alice:secretword@atlanta.com;transport=tcp
// sips:alice@atlanta.com?subject=project%20x&priority=urgent
// sip:+1-212-555-1212:1234@gateway.com;user=phone
// sips:1212@gateway.com
// sip:alice@192.0.2.4
// sip:atlanta.com;method=REGISTER?to=alice%40atlanta.com
// sip:alice;day=tuesday@atlanta.com

#include "sip-header.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int sip_header_uri(const char* s, const char* end, struct sip_uri_t* sip)
{
	const char* p;
	memset(sip, 0, sizeof(*sip));

	p = strchr(s, ':');
	if (!p || p >= end)
		return -1;
	
	sip->scheme.p = s;
	sip->scheme.n = p - s;
	sip->host.p = p + 1;

	s = p + 1;
	p = strpbrk(s, ";?");
	if (!p || p >= end)
	{
		sip->host.n = end - s;
	}
	else
	{
		sip->host.n = p - s;
		
		if (p && p < end && ';' == *p)
		{
			sip->parameters.p = p + 1;
			p = strpbrk(p + 1, "?");
			if(p && p < end)
				sip->parameters.n = p - sip->parameters.p;
			else
				sip->parameters.n = end - sip->parameters.p;
		}

		if (p && p < end && '?' == *p)
		{
			sip->headers.p = p + 1;
			sip->headers.n = end - p - 1;
		}
	}

	return 0;
}

// sip:user:password@host:port;uri-parameters?headers
int sip_uri_write(const struct sip_uri_t* uri, char* data, const char* end)
{
	char* p;
	if (!cstrvalid(&uri->scheme) || !cstrvalid(&uri->host))
		return -1; // error

	p = data;
	if (p < end) p += cstrcpy(&uri->scheme, p, end - p);
	if (p < end) *p++ = ':';
	if (p < end) p += cstrcpy(&uri->host, p, end - p);

	if (cstrvalid(&uri->parameters))
	{
		if (p < end) *p++ = ';';
		if (p < end) p += cstrcpy(&uri->parameters, p, end - p);
	}

	if (cstrvalid(&uri->headers))
	{
		if (p < end) *p++ = '?';
		if (p < end) p += cstrcpy(&uri->headers, p, end - p);
	}

	return p - data;
}

#if defined(DEBUG) || defined(_DEBUG)
void sip_uri_parse_test(void)
{
	char p[1024];
	const char* s;
	struct sip_uri_t uri;

	s = "sip:user:password@host:port;uri-parameters?headers";
	assert(0 == sip_header_uri(s, s + strlen(s), &uri));
	assert(0 == cstrcmp(&uri.scheme, "sip") && 0 == cstrcmp(&uri.host, "user:password@host:port") && 0 == cstrcmp(&uri.parameters, "uri-parameters") && 0 == cstrcmp(&uri.headers, "headers"));
	assert(sip_uri_write(&uri, p, p + sizeof(p)) < sizeof(p) && 0 == strcmp(s, p));

	s = "sip:alice@atlanta.com";
	assert(0 == sip_header_uri(s, s + strlen(s), &uri));
	assert(0 == cstrcmp(&uri.scheme, "sip") && 0 == cstrcmp(&uri.host, "alice@atlanta.com") && 0 == uri.parameters.n && 0 == uri.parameters.n);
	assert(sip_uri_write(&uri, p, p + sizeof(p)) < sizeof(p) && 0 == strcmp(s, p));

	s = "sips:alice@atlanta.com?subject=project%20x&priority=urgent";
	assert(0 == sip_header_uri(s, s + strlen(s), &uri));
	assert(0 == cstrcmp(&uri.scheme, "sips") && 0 == cstrcmp(&uri.host, "alice@atlanta.com") && 0 == uri.parameters.n && 0 == cstrcmp(&uri.headers, "subject=project%20x&priority=urgent"));
	assert(sip_uri_write(&uri, p, p + sizeof(p)) < sizeof(p) && 0 == strcmp(s, p));

	s = "sip:alice:secretword@atlanta.com;transport=tcp";
	assert(0 == sip_header_uri(s, s + strlen(s), &uri));
	assert(0 == cstrcmp(&uri.scheme, "sip") && 0 == cstrcmp(&uri.host, "alice:secretword@atlanta.com") && 0 == cstrcmp(&uri.parameters, "transport=tcp") && 0 == uri.headers.n);
	assert(sip_uri_write(&uri, p, p + sizeof(p)) < sizeof(p) && 0 == strcmp(s, p));

	s = "sip:+1-212-555-1212:1234@gateway.com;user=phone";
	assert(0 == sip_header_uri(s, s + strlen(s), &uri));
	assert(0 == cstrcmp(&uri.scheme, "sip") && 0 == cstrcmp(&uri.host, "+1-212-555-1212:1234@gateway.com") && 0 == cstrcmp(&uri.parameters, "user=phone") && 0 == uri.headers.n);
	assert(sip_uri_write(&uri, p, p + sizeof(p)) < sizeof(p) && 0 == strcmp(s, p));

	s = "sip:alice;day=tuesday@atlanta.com";
	assert(0 == sip_header_uri(s, s + strlen(s), &uri));
	assert(0 == cstrcmp(&uri.scheme, "sip") && 0 == cstrcmp(&uri.host, "alice") && 0 == cstrcmp(&uri.parameters, "day=tuesday@atlanta.com") && 0 == uri.headers.n);
	assert(sip_uri_write(&uri, p, p + sizeof(p)) < sizeof(p) && 0 == strcmp(s, p));

	s = "sip:p2.domain.com;lr";
	assert(0 == sip_header_uri(s, s + strlen(s), &uri));
	assert(0 == cstrcmp(&uri.scheme, "sip") && 0 == cstrcmp(&uri.host, "p2.domain.com") && 0 == cstrcmp(&uri.parameters, "lr") && 0 == uri.headers.n);
	assert(sip_uri_write(&uri, p, p + sizeof(p)) < sizeof(p) && 0 == strcmp(s, p));
}
#endif
