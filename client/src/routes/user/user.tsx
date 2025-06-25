import { useEffect, useRef, useState } from "react";
import { useParams } from "react-router-dom";
import { Card, CardHeader, CardSubHeader } from "~/components/card";
import { UserData } from "~/types";
import { UserController } from "~/utils/api";
import ProfilePicture from "~/components/profile-picture";

import styles from "./user.module.css";

const levelColors: Record<string, string> = {
  "352001527780474881": "",
  "350483752490631181": "#1528d6",
  "351117824300679169": "#1b9e52",
  "351117954974482435": "#179992",
  "350485376109903882": "#ba7011",
  "351118486426091521": "#ba1c2c",
  "350485279238258689": "#b01965",
  "350483489461895168": "#1b5eb3",
};

const levelMap: Record<string, string> = {
  "352001527780474881": "Non Learner",
  "350483752490631181": "Native",
  "351117824300679169": "Beginner",
  "351117954974482435": "Elementary",
  "350485376109903882": "Intermediate",
  "351118486426091521": "Upper Intermediate",
  "350485279238258689": "Advanced",
  "350483489461895168": "Fluent",
};

const User: React.FC = () => {
  const { userId } = useParams<{ userId: string }>();
  const userHeaderRef = useRef<HTMLDivElement>(null);

  const [user, setUser] = useState<UserData | null>(null);
  const [wrapped, setWrapped] = useState(false);
  const [wrapWidth, setWrapWidth] = useState<number | null>(null);

  /**
   * Fetches user data from the API based on the userId from the URL parameters.
   */
  const fetchUser = async () => {
    const userData = await UserController.getUser(userId!);
    if (userData) {
      setUser(userData.message[0]);
    } else {
      window.location.href = "/";
    }
  };

  /**
   *
   */
  const checkWrap = () => {
    if (userHeaderRef.current) {
      const profilePicture = userHeaderRef.current.querySelector(
        `.${styles.user_profile_picture}`
      ) as HTMLElement;
      const userDetail = userHeaderRef.current.querySelector(
        `.${styles.user_detail}`
      ) as HTMLElement;

      if (profilePicture && userDetail) {
        const profilePictureRect = profilePicture.getBoundingClientRect();
        const userDetailRect = userDetail.getBoundingClientRect();
        const isWrapped = userDetailRect.top > profilePictureRect.top;

        // Store the window width when wrap occurs for the first time
        if (isWrapped && !wrapped) {
          setWrapWidth(window.innerWidth);
        }

        // Update wrapped state based on current window width and stored wrap width
        if (wrapWidth !== null) {
          setWrapped(window.innerWidth <= wrapWidth);
        } else {
          setWrapped(isWrapped);
        }
      }
    }
  };

  useEffect(() => {
    checkWrap();

    window.addEventListener("resize", checkWrap);
    return () => {
      window.removeEventListener("resize", checkWrap);
    };
  }, [user, wrapped, wrapWidth]);

  useEffect(() => {
    if (!userId) {
      window.location.href = "/";
    }
    fetchUser();
  }, [userId]);

  if (!user) {
    return <div>Loading...</div>;
  }

  let levelId, levelName, levelColour;

  if (user.levels) {
    levelId = user.levels.find((lvl) => lvl in levelMap);
    levelName = levelId ? levelMap[levelId] : null;
    levelColour = levelId ? levelColors[levelId] : "";
  }

  return (
    <div className={styles.card_wrapper}>
      <Card className={styles.user_card}>
        <CardHeader
          ref={userHeaderRef}
          className={`${styles.user_header} ${
            wrapped ? styles.flex_wrap_active : ""
          }`}
        >
          <ProfilePicture
            className={styles.user_profile_picture}
            avatar={user.user.avatar}
            discord_id={user.user.discord_id}
            variant="large"
          />
          <div className={styles.user_detail}>
            <div className={styles.user_brief}>
              <span className={styles.user_name}>{user.user.nickname}</span>
              <CardSubHeader className={styles.user_info}>
                <span className={styles.user_name}>{user.user.username}</span>
                {levelName && (
                  <span style={{ color: levelColour }}>{levelName}</span>
                )}
              </CardSubHeader>
            </div>
            <CardSubHeader className={styles.user_footer}>
              <span className={styles.annotation_count}>
                {user.annotation_count} Annotations
              </span>
              <span>{user.like_count - user.dislike_count} Rating</span>
            </CardSubHeader>
          </div>
        </CardHeader>
      </Card>
    </div>
  );
};

export default User;
