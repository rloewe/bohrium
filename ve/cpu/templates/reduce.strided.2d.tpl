{
    {{#OPERAND}}
    {{TYPE}} *a{{NR}}_current = a{{NR}}_first;
    {{/OPERAND}}

    {{TYPE_AXIS}} axis = *a{{NR_SINPUT}}_first;
    {{TYPE_AXIS}} other_axis = (axis==0) ? 1 : 0;

    int64_t nelements   = a{{NR_FINPUT}}_shape[other_axis];
    int mthreads        = omp_get_max_threads();
    int64_t nworkers    = nelements > mthreads ? mthreads : 1;

    #pragma omp parallel for num_threads(nworkers)
    for(int64_t j=0; j<a{{NR_FINPUT}}_shape[other_axis]; ++j) {
        
        {{TYPE_INPUT}} *tmp_current = a{{NR_FINPUT}}_first + \
                                      a{{NR_FINPUT}}_stride[other_axis] * j;

        {{TYPE_INPUT}} rvar = *tmp_current;                   // Scalar-temp 
        for(int64_t i=1; i<a{{NR_FINPUT}}_shape[axis]; ++i) { // Accumulate
            tmp_current += a{{NR_FINPUT}}_stride[axis];

            {{#OPERATORS}}
            {{OPERATOR}};
            {{/OPERATORS}}
        }
        // Update array
        *(a{{NR_OUTPUT}}_first + a{{NR_OUTPUT}}_start + a{{NR_OUTPUT}}_stride[0]*j) = rvar; 
    }
}
