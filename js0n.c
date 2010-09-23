// by jeremie miller - 2010
// public domain, contributions/improvements welcome via github

// opportunity to further optimize would be having different jump tables for higher depths
#define PUSH(i) if(depth == 1) *out++ = ((cur+i) - js)
#define CAP(i) if(depth == 1) *out++ = (cur+i) - (js + *(out-1)) + 1

int js0n(unsigned char *js, unsigned int len, unsigned short *out)
{
	unsigned char *cur, *end;
	int depth=0;
    static void *gostruct[] = 
    {
        [0 ... 255] = &&l_bad,
        ['\t'] = &&l_ws, [' '] = &&l_ws, ['\r'] = &&l_ws, ['\n'] = &&l_ws,
        ['"'] = &&l_quot,
        [':'] = &&l_is,[','] = &&l_more,
		['['] = &&l_up, [']'] = &&l_down, // tracking [] and {} individually would allow fuller validation but is really messy
		['{'] = &&l_up, ['}'] = &&l_down,
		['-'] = &&l_bare, [48 ... 57] = &&l_bare, // 0-9
		['t'] = &&l_bare, ['f'] = &&l_bare, ['n'] = &&l_bare // true, false, null
    };
    static void *gobare[] = 
    {
    	[0 ... 31] = &&l_bad,
        [32 ... 126] = &&l_char,
        ['\t'] = &&l_unbare, [' '] = &&l_unbare, ['\r'] = &&l_unbare, ['\n'] = &&l_unbare,
        [','] = &&l_unbare, [']'] = &&l_unbare, ['}'] = &&l_unbare,
        [127 ... 255] = &&l_bad
    };
    static void *gostring[] = 
    {
    	[0 ... 31] = &&l_bad, [127] = &&l_bad,
        [32 ... 126] = &&l_char,
        ['\\'] = &&l_esc, ['"'] = &&l_quot,
        [128 ... 255] = &&l_char
    };
    static void *goesc[] = 
    {
        [0 ... 255] = &&l_bad,
        ['"'] = &&l_unesc, ['\\'] = &&l_unesc, ['/'] = &&l_unesc, ['b'] = &&l_unesc,
		['f'] = &&l_unesc, ['n'] = &&l_unesc, ['r'] = &&l_unesc, ['t'] = &&l_unesc, ['u'] = &&l_unesc
    };
	static void **go = gostruct;
	
	for(cur=js,end=js+len; cur<end; cur++)
	{
			goto *go[*cur];
			l_loop:;
	}
	
	return depth; // 0 if successful full parse, >0 for incomplete data
	
	l_bad:
		return 1;
	
	l_char:
		goto l_loop;
	
	l_up:
		PUSH(0);
		++depth;
		goto l_loop;

	l_down:
		--depth;
		CAP(0);
		goto l_loop;

	l_ws:
	l_is:
	l_more:
		goto l_loop;
	
	l_quot:
		if(go==gostruct)
		{
			PUSH(1);
			go=gostring;
		}else{
			CAP(-1);
			go=gostruct;
		}
		goto l_loop;
		
	l_esc:
		go = goesc;
		goto l_loop;
		
	l_unesc:
		go = gostring;
		goto l_loop;

	l_bare:
		PUSH(0);
		go = gobare;
		goto l_loop;

	l_unbare:
		CAP(-1);
		go = gostruct;
		goto *go[*cur];
		
}
