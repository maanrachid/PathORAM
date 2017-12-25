#ifndef MyMath_h
#define MyMath_h


void path_level(int leaf,int level,int *min,int *max){
  int power_of_2=1;
  for(int i=0;i<level;i++)
    power_of_2 *=2;
  *min = leaf/power_of_2 * power_of_2;
  *max = *min + power_of_2 -1;
}

int log(int n){
  int temp=1;
  int base =2;
  int counter=0;
  while (temp<n){
    temp=temp*base;
    counter++;
  }
  return counter;
}

int Sqrt(int x)
{
    // Base cases
    if (x == 0 || x == 1)
       return x;
 
    // Staring from 1, try all numbers until
    // i*i is greater than or equal to x.
    int i = 1, result = 1;
    while (result < x)
    {
       if (result == x)
          return result;
       i++;
       result = i*i;
    }
    return i-1;
}

int find_closet_power_of_two(int size){
  int i=2;
  while(i<size)
    i*=2;
  return i;
}


#endif
