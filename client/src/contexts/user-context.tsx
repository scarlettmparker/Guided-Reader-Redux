import {
  createContext,
  useContext,
  useState,
  useEffect,
  ReactNode,
} from "react";
import { UserController } from "~/utils/api";

interface UserData {
  username: string;
  [key: string]: any;
}

interface UserContextType {
  user: UserData | null;
  setUser: (user: UserData | null) => void;
  logout: () => Promise<void>;
}

const UserContext = createContext<UserContextType | undefined>(undefined);

let cache: { user: UserData; expires: number } | null = null;

export function UserProvider({ children }: { children: ReactNode }) {
  const [user, setUser] = useState<UserData | null>(null);

  useEffect(() => {
    let cancelled = false;
    async function loadUser() {
      const now = Date.now();
      if (cache && cache.expires > now) {
        setUser(cache.user);
        return;
      }
      try {
        const data = await UserController.get();
        if (!cancelled) {
          setUser(data.message || data);
          cache = { user: data.message || data, expires: now + 5 * 60 * 1000 };
        }
      } catch {
        setUser(null);
        cache = null;
        await UserController.logout();
      }
    }
    loadUser();
    return () => {
      cancelled = true;
    };
  }, []);

  const logout = async () => {
    await UserController.logout();
    setUser(null);
    cache = null;
  };

  return (
    <UserContext.Provider value={{ user, setUser, logout }}>
      {children}
    </UserContext.Provider>
  );
}

export function useUser() {
  const ctx = useContext(UserContext);
  if (!ctx) throw new Error("useUser must be used within a UserProvider");
  return ctx;
}
