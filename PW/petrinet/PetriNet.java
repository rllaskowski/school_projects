package petrinet;

import java.util.*;
import java.util.concurrent.Semaphore;

public class PetriNet<T> {
    private Map<T, Integer> places;
    private boolean fair;
    private final Map<T, Semaphore> semaphores = new HashMap<>();
    private final Semaphore fireSem;
    private final ArrayList<Semaphore> sleepingThreads = new ArrayList<>();
    private int sleepingIt;

    public PetriNet(Map<T, Integer> initial, boolean fair) {
        this.places = Collections.synchronizedMap(initial);
        this.fair = fair;
        fireSem = new Semaphore(1, fair);
        sleepingIt = 0;
    }

    public Set<Map<T, Integer>> reachable(Collection<Transition<T>> transitions) {
        Set<Map<T, Integer>> result = new HashSet<>();

        HashMap<T, Integer> firstResult = new HashMap<T, Integer>(places);
        firstResult.values().removeIf(f -> f == 0);
        result.add(firstResult);

        reachableRec(result, transitions);

        return result;
    }

    private boolean setContainsMap(Set<Map<T, Integer>> set,  Map<T, Integer> map) {
        for (Map<T, Integer> x : set) {
            if (x.equals(map)) {
                return true;
            }
        }
        return false;
    }

    private void reachableRec(Set<Map<T, Integer>> results, Collection<Transition<T>> transitions) {
        for (Map<T, Integer> result : results) {
            for (Transition<T> transition : transitions) {
                Map<T, Integer> tempResult = new HashMap<>(result);

                if (fireTransitionOn(transition, tempResult)) {
                    tempResult.values().removeIf(v -> v == 0);

                    if (!results.contains(tempResult)) {
                        results.add(tempResult);

                        reachableRec(results, transitions);
                    }
                }
            }
        }
    }

    private boolean isActiveOn(Transition<T> transition, Map<T, Integer> places) {
        for (Map.Entry<T, Integer> edgeEntry : transition.getInput().entrySet()) {
            if (edgeEntry.getValue() > places.getOrDefault(edgeEntry.getKey(), 0)) {
                return false;
            }
        }
        for (T place : transition.getInhibitor()) {
            if (places.getOrDefault(place, 0) != 0) {
                return false;
            }
        }
        return true;
    }

    private boolean fireTransitionOn(Transition<T> transition, Map<T, Integer> places) {
        boolean fired = false;

        if (isActiveOn(transition, places)) {
            for (Map.Entry<T, Integer> edgeEntry : transition.getInput().entrySet()) {
                places.compute(edgeEntry.getKey(), (key, value) -> value-edgeEntry.getValue());
            }

            for (T place : transition.getReset()) {
                places.computeIfPresent(place, (key, value) -> 0);
            }

            for (Map.Entry<T, Integer> edgeEntry : transition.getOutput().entrySet()) {
                places.putIfAbsent(edgeEntry.getKey(), 0);
                places.compute(edgeEntry.getKey(), (key, value) -> value + edgeEntry.getValue());
            }

            fired = true;
        }

        return fired;
    }

    public Transition<T> fire(Collection<Transition<T>> transitions) throws InterruptedException {
        boolean repeat = true;
        Transition<T> resultTransition = null;

        fireSem.acquire();
        Semaphore sleepingSem = new Semaphore(1);

        while (repeat) {
            sleepingSem.acquire();

            for (Transition<T> transition : transitions) {
                if (isActiveOn(transition, this.places)) {
                    resultTransition = transition;
                    repeat = false;
                    break;
                }
            }

            if (repeat == true) {
                if (sleepingThreads.contains(sleepingSem)) {
                    if (sleepingIt+1 < sleepingThreads.size()) {
                        sleepingIt += 1;
                        sleepingThreads.get(sleepingIt).release();
                    } else {
                        fireSem.release();
                    }

                } else {
                    sleepingThreads.add(sleepingSem);
                    fireSem.release();
                }
            } else {
                fireTransitionOn(resultTransition, places);

                if (sleepingThreads.contains(sleepingSem)) {
                    sleepingThreads.remove(sleepingSem);
                }

                if (sleepingThreads.size() > 0) {
                    if (!this.fair) {
                        Collections.shuffle(sleepingThreads);
                    }

                    sleepingIt = 0;
                    sleepingThreads.get(0).release();
                } else {
                    fireSem.release();
                }
                break;
            }
        }

        return resultTransition;
    }
}