import { createContext, useContext } from "react";

type TextContextType = {
  setSelectedTextId: (id: number) => void;
};

export const TextContext = createContext<TextContextType | undefined>(
  undefined,
);

export const useTextContext = () => {
  const context = useContext(TextContext);
  if (!context) {
    throw new Error("useTextContext must be used within a TextContextProvider");
  }
  return context;
};
