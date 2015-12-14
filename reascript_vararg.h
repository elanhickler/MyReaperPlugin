static void* __vararg_AddTwoNumbers(void** arglist, int numparms)
{
  return (void*)(INT_PTR)AddTwoNumbers((int)(INT_PTR)arglist[0], (int)(INT_PTR)arglist[1]);
}

