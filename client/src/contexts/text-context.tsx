import { createContext, useContext } from "solid-js";

type TextContextType = {
  setSelectedTextId: (id: number) => void;
};

export const TextContext = createContext<TextContextType>();

export function useTextContext() {
  const context = useContext(TextContext);
  if (!context) {
    throw new Error("useTextContext must be used within a TextContextProvider");
  }
  return context;
}