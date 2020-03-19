package petrinet;

import java.util.Collection;
import java.util.Map;


public class Transition<T> {
    private Map<T, Integer> input;
    private Collection<T> reset;
    private Collection<T> inhibitor;
    private Map<T, Integer> output;

    public Transition(Map<T, Integer> input, Collection<T> reset, Collection<T> inhibitor, Map<T, Integer> output) {
        this.input = input;
        this.reset = reset;
        this.inhibitor = inhibitor;
        this.output = output;
    }

    public Map<T, Integer> getInput() {
        return input;
    }

    public Collection<T> getReset() {
        return reset;
    }

    public Collection<T> getInhibitor() {
        return inhibitor;
    }

    public Map<T, Integer> getOutput() {
        return output;
    }
}