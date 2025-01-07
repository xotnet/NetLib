#include <stdio.h>
#include "net.c"

int main() {
	printf("Enter domain name (example: srv.slowdns.org): ");
	char domain[128];
	scanf("%[^\n]s", domain);
	printf("Select NS lookup type\n[1] type A\n[2] type SRV\n[3] type CAA\n");
	while(getchar() != '\n');
	int type;
	scanf("%d", &type);
	if (type == 2) {type = dnsSRV;}
	else if (type == 3) {type = dnsCAA;}

	// USING NETLIB
	char result[128];
	resolve_net(domain, result, type);
	printf("DNS record of %s : %s\n", domain, result);
}
