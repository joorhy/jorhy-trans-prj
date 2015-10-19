#ifndef __X_BASE64_H_
#define __X_BASE64_H_

#ifdef __cplusplus
extern "C"
{
#endif
	extern void base64_in(unsigned char *buf, char *obuf, int len);
	extern void base64_out(char *buf, unsigned char *obuf, int len);

#ifdef __cplusplus
};
#endif
#endif //~__X_BASE64_H_