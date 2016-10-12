// Convert 30-bit and MPZ numbers to 32-bit hexa strings for comparisons
// (c) EB Nov 2009

#ifndef Conversions_h
#define Conversions_h

// Convert X[N] from redundand representation in base 2^30
// to an hex string.
void convert30(int n,const int * x,std::string & out)
{
  int nAux = 0; // Valid bits in aux
  __int64 aux = 0;
  std::vector<unsigned int> y(n,0);
  int j = 0; // Next value in Y receiving bits

  for (int i=0;i<n;i++)
  {
    // Insert X[i] into AUX
    aux += ((__int64)x[i]<<nAux);
    nAux += 30;
    // Extract 32 bits if available
    if (nAux>=32)
    {
      y[j++] = (unsigned int)(aux&0xFFFFFFFF);
      aux >>= 32;
      nAux -= 32;
    }
  }
  // Extract last bits
  while (nAux > 0)
  {
    y[j++] = (unsigned int)(aux&0xFFFFFFFF);
    aux >>= 32;
    nAux -= 32;
  }
  out.clear();
  out.reserve(9*n+20);
  char s[32];
  bool first = true;
  for (int i=n-1;i>=0;i--)
  {
    if (first)
    {
      if (y[i] == 0) continue;
      first = false;
    }
    else out.push_back('.');
    _snprintf(s,32,"%08X",y[i]);
    out.append(s);
  }
  if (first) out.append("0");
}

#if CONFIG_USE_MPIR
// Convert mpz_t integer to a string
void convertMPZ(mpz_t x,std::string & out)
{
  char * s = mpz_get_str(0,16,x);
  if (s == 0) return; // Failed
  int n = (int)strlen(s);
  out.clear();
  out.reserve(n+n/8+8);
  int digits = 0;
  while ( (digits+n)&7 ) { out.push_back('0'); digits++; } // Initial 0 to get a multiple of 8
  for (int i=0;i<n;i++)
  {
    if ( (digits&7) == 0 && i>0 ) out.push_back('.');
    out.push_back(toupper(s[i])); digits++;
  }
  free(s);
}
#endif

#endif // Conversions_h
